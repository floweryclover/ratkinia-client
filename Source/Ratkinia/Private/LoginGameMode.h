// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"

#include "CoreMinimal.h"
#include "CtsProxy.gen.h"
#include "GameFramework/GameModeBase.h"
#include "LoginGameMode.generated.h"

class URegisterWidget;
class ULoginWidget;
class UMessageBoxWidget;
/**
 * 
 */
UCLASS()
class ALoginGameMode final : public AGameModeBase, public RatkiniaProtocol::StcStub<ALoginGameMode>
{
	GENERATED_BODY()
	
public:
	explicit ALoginGameMode();
	
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnUnknownMessageType(uint32_t context, RatkiniaProtocol::StcMessageType messageType) override;

	virtual void OnParseMessageFailed(uint32_t context, RatkiniaProtocol::StcMessageType messageType) override;
	
	virtual void OnUnhandledMessageType(uint32_t context, RatkiniaProtocol::StcMessageType messageType) override;

	virtual void OnLoginResponse(uint32_t context, const bool successful) override;

	virtual void OnRegisterResponse(uint32_t context, const bool successful, const std::string& failed_reason) override;
	
protected:
	virtual void BeginPlay() override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMessageBoxWidget> MessageBoxWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULoginWidget> LoginWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URegisterWidget> RegisterWidgetClass;
	
	UFUNCTION()
	void RatkiniaLogin(FText Id, FText Password);

	UFUNCTION()
	void RatkiniaRegister(FText Id, FText Password, FText PasswordAgain);
	
	UFUNCTION()
	void OpenLoginWidget();

	UFUNCTION()
	void OpenRegisterWidget();

	UFUNCTION()
	void OpenResetPasswordWidget();

	UFUNCTION()
	void OpenReallyQuitGameWidget();
	
	void PopupMessageBoxWidget(FText Text);

public:

private:
	TFunction<void()> PostConnectAction;
};
