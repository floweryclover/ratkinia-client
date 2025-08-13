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

	virtual void OnUnknownMessageType(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnParseMessageFailed(RatkiniaProtocol::StcMessageType MessageType) override;
	
	virtual void OnUnhandledMessageType(RatkiniaProtocol::StcMessageType MessageType) override;

	virtual void OnLoginResponse(const RatkiniaProtocol::LoginResponse_LoginResult Result) override;

	virtual void OnRegisterResponse(const bool Successful, FString FailedReason) override;
	
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

private:
	TFunction<void()> PostConnectAction;
};
