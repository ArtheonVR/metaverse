// Copyright © 2021++ Ready Player Me

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyPlayerMeTypes.h"
#include "glTFRuntimeFunctionLibrary.h"
#include "ReadyPlayerMeComponent.generated.h"

class USkeleton;
class UglTFRuntimeAsset;

/**
 * responsible for the loading of the avatar and visualizing it by setting the SkeletalMesh of the avatar.
 * It also provides useful functions for loading the avatar and loading a rendered image of the avatar.
 *
 * @see ReadyPlayerMeAvatarLoader
 * @see ReadyPlayerMeRenderLoader
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class READYPLAYERME_API UReadyPlayerMeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Default constructor. */
	UReadyPlayerMeComponent();

	/** 
	 * Downloads the avatar from the web and generates a skeletal mesh of the avatar.
	 * If the model was previously downloaded and stored locally, the local model will be used for the loading of the avatar. 
	 * 
	 * @param OnLoadFailed Failure callback. If the avatar fails to load, the failure callback will be called.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ready Player Me", meta = (DisplayName = "Load Avatar", AutoCreateRefTerm = "OnLoadFailed"))
	void LoadAvatar(const FAvatarLoadFailed& OnLoadFailed);

	/**
	 * Downloads the avatar from the web using the provided url and generates a skeletal mesh of the avatar.
	 * If the model was previously downloaded and stored locally, the local model will be used for the loading of the avatar. 
	 * 
	 * @param Url Avatar url or shortcode.
	 * @param OnLoadFailed Failure callback. If the avatar fails to load, the failure callback will be called.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ready Player Me", meta = (DisplayName = "Load New Avatar", AutoCreateRefTerm = "OnLoadFailed"))
	void LoadNewAvatar(const FString& Url, const FAvatarLoadFailed& OnLoadFailed);

	/**
	 * Loads the rendered image of the avatar from the server. By setting the SceneType the avatar can be rendered in different scenes.
	 * 
	 * @param SceneType The type of the scene where the avatar should be rendered.
	 * @param OnCompleted Success callback. Called when the render is loaded and provides the avatar texture as an argument.
	 * @param OnFailed Failure callback. If the render operation fails, the failure callback will be called.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ready Player Me", meta = (DisplayName = "Load Render", AutoCreateRefTerm = "OnCompleted, OnFailed"))
	void LoadRender(const ERenderSceneType& SceneType, const FDownloadImageCompleted& OnCompleted, const FDownloadImageFailed& OnFailed);

	/** The avatar url or shortcode. It's used to load the avatar from the web. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ready Player Me")
	FString UrlShortcode;

	/** provides read-only information about the loaded avatar. Such as the type of the avatar, outfit and gender. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ready Player Me")
	FAvatarMetadata AvatarMetadata;

	/** It provides a flexibility to chose the skeleton that will be used for the loaded avatar.
	 * If it's not set the default skeleton will be used for the loaded avatar. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ready Player Me")
	USkeleton* TargetSkeleton;

private:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY()
	class UReadyPlayerMeAvatarLoader* AvatarLoader;

	UPROPERTY()
	class UReadyPlayerMeRenderLoader* RenderLoader;

	UFUNCTION()
	void OnAvatarLoaded(UglTFRuntimeAsset* Asset, const FAvatarMetadata& Metadata);

	UFUNCTION()
	void OnSkeletalMeshLoaded(USkeletalMesh* SkeletalMesh);

	void InitSkeletalMeshComponent();

	void LoadSkeletalMesh(UglTFRuntimeAsset* Asset);

	FAvatarLoadCompleted OnAvatarLoadCompleted;

	FglTFRuntimeSkeletalMeshAsync OnSkeletalMeshCallback;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAvatarLoadCompletedDelegate);

	UPROPERTY(BlueprintAssignable)
	FOnAvatarLoadCompletedDelegate OnAvatarLoadCompletedDelegate;
};
