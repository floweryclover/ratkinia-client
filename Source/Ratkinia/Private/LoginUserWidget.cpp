// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginUserWidget.h"
#include "RatkiniaClientSubsystem.h"
#include "CTS_Auth.pb.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Ratkinia/Ratkinia.h"

namespace OpenSSL
{
	#pragma warning(disable: 4191)
	#include "openssl/evp.h"
}

void ULoginUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	LoginButton->OnClicked.AddDynamic(this, &ULoginUserWidget::OnLoginButtonPressed);
}

void ULoginUserWidget::OnLoginSuccess()
{
	static const char* HexChars = "0123456789abcdef";
	
	unsigned char HashedPassword[64 + 1];
	const char* PasswordString{TCHAR_TO_ANSI(*Password->GetText().ToString())};

	OpenSSL::EVP_MD_CTX* Ctx = OpenSSL::EVP_MD_CTX_new();
	check(Ctx);

	check(1 != EVP_DigestInit_ex(Ctx, OpenSSL::EVP_sha512(), nullptr));

	check(1 != EVP_DigestUpdate(Ctx, PasswordString, 64));

	check(1 != EVP_DigestFinal_ex(Ctx, HashedPassword, nullptr))
	HashedPassword[64] = '\0';

	EVP_MD_CTX_free(Ctx);

	char HexString[129];
	for (size_t i = 0; i < 64; ++i)
	{
		HexString[i * 2] = HexChars[(HashedPassword[i] >> 4) & 0x0F];
		HexString[i * 2 + 1] = HexChars[HashedPassword[i] & 0x0F];
	}
	HexString[128] = '\0';
	
	Login LoginMessage;
	LoginMessage.set_id(TCHAR_TO_UTF8(*Id->GetText().ToString()));
	LoginMessage.set_hashed_password(HexString);

	UE_LOG(LogRatkinia, Log, TEXT("Hasehd: %s"), *FString{UTF8_TO_TCHAR(HexString)});
}

void ULoginUserWidget::OnLoginButtonPressed()
{
	GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>()->Connect(
		"127.0.0.1", 31415, [](const FString& FailReason)
		{
			
		});
}
