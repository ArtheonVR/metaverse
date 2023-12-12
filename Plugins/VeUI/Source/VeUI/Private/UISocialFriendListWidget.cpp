﻿// Authors: Egor A. Pristavka, Nikolay Bulatov
// Copyright (c) 2022-2023 VeVerse AS. All rights reserved.
// Copyright (c) 2023 LE7EL AS. All rights reserved.

#include "UISocialFriendListWidget.h"

#include "Api2UserSubsystem.h"
#include "VeApi.h"
#include "VeUI.h"
#include "ApiUserMetadataObject.h"
#include "UIPageManagerWidget.h"
#include "VeApi2.h"
#include "Subsystems/UIPageSubsystem.h"
#include "Subsystems/UINotificationSubsystem.h"

#define LOCTEXT_NAMESPACE "VeUI"


void UUISocialFriendListWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (PaginationWidget) {
		PaginationWidget->ItemsPerPage = ItemsPerPage;
	}

	RequestMetadata.Limit = ItemsPerPage;
	RequestMetadata.Offset = 0;
}

void UUISocialFriendListWidget::RegisterCallbacks() {
	Super::RegisterCallbacks();

	if (!OnContainerItemClickedDelegateHandle.IsValid()) {
		if (IsValid(ContainerWidget)) {
			OnContainerItemClickedDelegateHandle = ContainerWidget->OnItemClicked().AddWeakLambda(this, [this](UObject* InObject) {
				if (auto* MetadataObject = Cast<UApiUserMetadataObject>(InObject)) {
					GET_MODULE_SUBSYSTEM(PageSubsystem, UI, Page);
					if (PageSubsystem) {
						if (auto* PageManager = PageSubsystem->GetPageManagerUI()) {
							PageManager->OpenPage(DetailPageClass, *MetadataObject->Metadata->Id.ToString());
						}
					}
				}
			});
		}
	}

	if (!OnFilterChangeDelegateHandle.IsValid()) {
		if (FilterWidget) {
			OnFilterChangeDelegateHandle = FilterWidget->OnFilterChanged.AddWeakLambda(this, [this](FString InQuery) {
				RequestMetadata.Query = InQuery;
				Refresh();
			});
		}
	}

	if (!OnPaginationPageChangeDelegateHandle.IsValid()) {
		if (PaginationWidget) {
			OnPaginationPageChangeDelegateHandle = PaginationWidget->OnPageChanged.AddWeakLambda(this, [this](const int32 InCurrentPage, const int32 InItemsPerPage) {
				RequestMetadata.Limit = InItemsPerPage;
				RequestMetadata.Offset = InCurrentPage * InItemsPerPage;
				Refresh();
			});
		}
	}
}

void UUISocialFriendListWidget::UnregisterCallbacks() {
	Super::UnregisterCallbacks();

	if (OnContainerItemClickedDelegateHandle.IsValid()) {
		if (IsValid(ContainerWidget)) {
			ContainerWidget->OnItemClicked().Remove(OnContainerItemClickedDelegateHandle);
		}
		OnContainerItemClickedDelegateHandle.Reset();
	}

	if (OnFilterChangeDelegateHandle.IsValid()) {
		if (FilterWidget) {
			FilterWidget->OnFilterChanged.Remove(OnFilterChangeDelegateHandle);
		}
		OnFilterChangeDelegateHandle.Reset();
	}

	if (OnPaginationPageChangeDelegateHandle.IsValid()) {
		if (PaginationWidget) {
			PaginationWidget->OnPageChanged.Remove(OnPaginationPageChangeDelegateHandle);
		}
		OnPaginationPageChangeDelegateHandle.Reset();
	}
}

void UUISocialFriendListWidget::SetUserId(const FGuid& InUserGuid) {
	const bool changed = UserId != InUserGuid;
	UserId = InUserGuid;
	if (changed) {
		Refresh();
	}
}

void UUISocialFriendListWidget::Refresh() {
	RequestFriends(RequestMetadata);
}

void UUISocialFriendListWidget::RequestFriends(const IApiBatchQueryRequestMetadata& InRequestMetadata) {
	GET_MODULE_SUBSYSTEM(AuthSubsystem, Api, Auth);
	if (AuthSubsystem) {
		if (!AuthSubsystem->IsLoggedIn()) {
			return;
		}
	}

	if (!UserId.IsValid()) {
		return;
	}

	GET_MODULE_SUBSYSTEM(UserSubsystem, Api2, User);
	if (UserSubsystem) {
		// 1. Create a callback.
		TSharedPtr<FOnUserBatchRequestCompleted2> CallbackPtr = MakeShareable(new FOnUserBatchRequestCompleted2());

		// 2. Bind callback.
		CallbackPtr->BindWeakLambda(this, [=](const FApiUserBatchMetadata& InMetadata, const EApi2ResponseCode InResponseCode, const FString& InError) mutable {
			const bool bInSuccessful = InResponseCode == EApi2ResponseCode::Ok;
			if (!bInSuccessful) {
				LogErrorF("failed to request: %s", *InError);
			}

			// Finish processing.
			SetIsProcessing(false);

			OnFriendsLoaded(InMetadata, bInSuccessful, InError);

			CallbackPtr.Reset();
		});

		// 3. Make the request using the callback.
		UserSubsystem->GetFriends(UserId, InRequestMetadata, CallbackPtr);
	}
}

void UUISocialFriendListWidget::SetIsProcessing(bool bInIsLoading) {}

void UUISocialFriendListWidget::OnFriendsLoaded(const FApiUserBatchMetadata& Metadata, bool bSuccessful, const FString& Error) {
	if (bSuccessful) {
		if (IsValid(ContainerWidget)) {
			ContainerWidget->ScrollToTop();
			ContainerWidget->SetListItems(UApiUserMetadataObject::BatchToUObjectArray(Metadata.Entities, this));
		}
		if (IsValid(PaginationWidget)) {
			PaginationWidget->SetPaginationOptions(Metadata.Total, ItemsPerPage);
		}
	} else {
		// GET_MODULE_SUBSYSTEM(AuthSubsystem, Api, Auth);
		// if (AuthSubsystem) {
		// 	if (AuthSubsystem->IsLoggedIn()) {
		// 		GET_MODULE_SUBSYSTEM(NotificationSubsystem, UI, Notification);
		// 		if (NotificationSubsystem) {
		// 			FUINotificationData NotificationData;
		// 			NotificationData.Type = EUINotificationType::Failure;
		// 			NotificationData.Header = LOCTEXT("LoadUserBatchError", "Error");
		// 			NotificationData.Message = FText::FromString("Failed to load a friend batch.");
		// 			NotificationSubsystem->AddNotification(NotificationData);
		// 		}
		// 	}
		// }
		LogWarningF("failed to load friends");
	}
}


#undef LOCTEXT_NAMESPACE
