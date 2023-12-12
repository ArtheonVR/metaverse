﻿// Author: Egor A. Pristavka, Nikolay Bulatov
// Copyright (c) 2022 LE7EL AS. All right reserved.
#pragma once

#include "Components/UIInputWidget.h"
#include "UIWidget.h"
#include "UIMetaverseFilterWidget.generated.h"


/**
 * 
 */
UCLASS()
class VEUI_API UUIMetaverseFilterWidget : public UUIWidget {
	GENERATED_BODY()

	virtual void RegisterCallbacks() override;
	virtual void UnregisterCallbacks() override;

	FDelegateHandle OnSearchTextChangedDelegateHandle;
	FDelegateHandle OnButtonClickedDelegateHandle;
	
public:
	DECLARE_EVENT_OneParam(UUIModFilterWidget, FOnFilterChanged, const FString&);
	FOnFilterChanged OnFilterChanged;

	UPROPERTY(BlueprintReadOnly, Category="Widget|Content", meta=(BindWidget))
	UUIInputWidget* ModNameFilterWidget;

	UPROPERTY(BlueprintReadOnly, Category="Widget|Content", meta=(BindWidget))
	UUIButtonWidget* SearchButtonWidget;
};
