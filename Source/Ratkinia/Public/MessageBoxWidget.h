// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MessageBoxWidget.generated.h"

class UTextBlock;
class UButton;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOkButtonPressed);

/**
 * 
 */
UCLASS()
class RATKINIA_API UMessageBoxWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FOkButtonPressed OnOkButtonPressed;

	void SetMessage(FText InMessage);
	
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> OkButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Message;

private:
	UFUNCTION()
	void OnOkButtonPressedHandler();
};
