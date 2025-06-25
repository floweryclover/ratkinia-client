// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "StcStub.gen.h"

#include "CoreMinimal.h"
#include "CtsProxy.gen.h"
#include "GameFramework/GameModeBase.h"
#include "LoginGameMode.generated.h"

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

	virtual void OnUnknownMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messageType) override;

	virtual void OnParseMessageFailed(uint64_t context, RatkiniaProtocol::StcMessageType messageType) override;
	
	virtual void OnUnhandledMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messageType) override;

	virtual void OnLoginResponse(uint64_t context, const bool successful, const std::string& failure_reason) override;

	virtual void OnRegisterResponse(uint64_t context, const RatkiniaProtocol::RegisterResponse_FailedReason failed_reason) override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMessageBoxWidget> MessageBoxWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULoginWidget> LoginWidgetClass;
	
private:
	UPROPERTY()
	TObjectPtr<ULoginWidget> LoginWidget;
	
	FText RequestedLoginId;

	FText RequestedLoginPassword;

	bool bLoginRequested{};

	void PopupMessageBoxWidget(FText Text);

	UFUNCTION()
	void ConnectAndLogin(FText Id, FText Password);

	UFUNCTION()
	void OpenRegisterWidget();

	UFUNCTION()
	void OpenResetPasswordWidget();

	UFUNCTION()
	void OpenReallyQuitGameWidget();
};
