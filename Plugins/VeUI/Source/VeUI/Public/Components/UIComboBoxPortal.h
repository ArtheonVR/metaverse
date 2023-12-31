﻿// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"
#include "Widgets/Input/SComboBox.h"
#include "Components/Widget.h"
#include "VePortalMetadata.h"

#include "UIComboBoxPortal.generated.h"

/**
 * The combobox allows you to display a list of options to the user in a dropdown menu for them to select one.
 */
UCLASS(meta=( DisplayName="ComboBox (Portal)"))
class VEUI_API UUIComboBoxPortal : public UWidget
{
	GENERATED_UCLASS_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectionChangedEvent, FVePortalMetadata, SelectedItem, ESelectInfo::Type, SelectionType);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOpeningEvent);

private:

	/** The default list of items to be displayed on the combobox. */
	UPROPERTY(EditAnywhere, Category=Content)
	TArray<FVePortalMetadata> DefaultOptions;

	/** The item in the combobox to select by default */
	UPROPERTY(EditAnywhere, Category=Content)
	FVePortalMetadata SelectedOption;

public:

	/** The style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Style, meta=( DisplayName="Style" ))
	FComboBoxStyle WidgetStyle;

	/** The item row style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Style)
	FTableRowStyle ItemStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content)
	FMargin ContentPadding;

	/** The max height of the combobox list that opens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	float MaxListHeight;

	/**
	 * When false, the down arrow is not generated and it is up to the API consumer
	 * to make their own visual hint that this is a drop down.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	bool HasDownArrow;

	/**
	* When false, directional keys will change the selection. When true, ComboBox 
	* must be activated and will only capture arrow input while activated.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	bool EnableGamepadNavigationMode;

	/**
	 * The default font to use in the combobox, only applies if you're not implementing OnGenerateWidgetEvent
	 * to factory each new entry.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Style)
	FSlateFontInfo Font;

	/** The foreground color to pass through the hierarchy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Style, meta=(DesignerRebuild))
	FSlateColor ForegroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Interaction)
	bool bIsFocusable;

public: // Events

	/** Called when the widget is needed for the item. */
	UPROPERTY(EditAnywhere, Category=Events, meta=( IsBindableEvent="True" ))
	FGenerateWidgetForString OnGenerateWidgetEvent;

	/** Called when a new item is selected in the combobox. */
	UPROPERTY(BlueprintAssignable, Category=Events)
	FOnSelectionChangedEvent OnSelectionChanged;

	/** Called when the combobox is opening */
	UPROPERTY(BlueprintAssignable, Category=Events)
	FOnOpeningEvent OnOpening;

public:

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	void AddOption(const FVePortalMetadata& Option);

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	bool RemoveOption(const FGuid& Id);

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	int32 FindOptionIndex(const FGuid& Id) const;

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	FVePortalMetadata GetOptionAtIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	void ClearOptions();

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	void ClearSelection();

	/**
	 * Refreshes the list of options.  If you added new ones, and want to update the list even if it's
	 * currently being displayed use this.
	 */
	UFUNCTION(BlueprintCallable, Category="ComboBox")
	void RefreshOptions();

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	void SetSelectedOption(const FGuid& Id);

	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectedIndex(const int32 Index);

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	FVePortalMetadata GetSelectedOption() const;

	UFUNCTION(BlueprintCallable, Category="ComboBox")
	int32 GetSelectedIndex() const;

	/** Returns the number of options */
	UFUNCTION(BlueprintCallable, Category="ComboBox")
	int32 GetOptionCount() const;
	
	UFUNCTION(BlueprintCallable, Category="ComboBox", Meta = (ReturnDisplayName = "bOpen"))
	bool IsOpen() const;

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	//~ End UObject Interface

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	/** Refresh ComboBoxContent with the correct widget/data when the selected option changes */
	void UpdateOrGenerateWidget(TSharedPtr<FVePortalMetadata> Item);

	/** Called by slate when it needs to generate a new item for the combobox */
	virtual TSharedRef<SWidget> HandleGenerateWidget(TSharedPtr<FVePortalMetadata> Item) const;

	/** Called by slate when the underlying combobox selection changes */
	virtual void HandleSelectionChanged(TSharedPtr<FVePortalMetadata> Item, ESelectInfo::Type SelectionType);

	/** Called by slate when the underlying combobox is opening */
	virtual void HandleOpening();

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

protected:
	/** The true objects bound to the Slate combobox. */
	TArray< TSharedPtr<FVePortalMetadata> > Options;

	/** A shared pointer to the underlying slate combobox */
	TSharedPtr< SComboBox< TSharedPtr<FVePortalMetadata> > > MyComboBox;

	/** A shared pointer to a container that holds the combobox content that is selected */
	TSharedPtr< SBox > ComboBoxContent;

	/** If OnGenerateWidgetEvent is not bound, this will store the default STextBlock generated */
	TWeakPtr<STextBlock> DefaultComboBoxContent;

	/** A shared pointer to the current selected string */
	TSharedPtr<FVePortalMetadata> CurrentOptionPtr;
};
