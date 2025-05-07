// Copyright floweryclover @ SolarFlora 2025. All rights reserved.


#include "LoginGameMode.h"
#include "RatkiniaClientSubsystem.h"
#include "Ratkinia/Ratkinia.h"

namespace OpenSSL
{
#pragma warning(disable: 4191)
#include "openssl/evp.h"
}


ALoginGameMode::ALoginGameMode()
	: MessageBodyBuffer{MakeUnique<char[]>(RatkiniaProtocol::MessageMaxSize)}
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

ALoginGameMode::~ALoginGameMode()
{
}

void ALoginGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	URatkiniaClientSubsystem* const RatkiniaClient = GetGameInstance()->GetSubsystem<URatkiniaClientSubsystem>();

	if (!RatkiniaClient->IsConnected())
	{
		return;
	}

	if (RatkiniaClient->IsStopped())
	{
		UE_LOG(LogRatkinia, Error, TEXT("%s"), *RatkiniaClient->GetStoppedReason());
		RatkiniaClient->Disconnect();
		return;
	}

	constexpr const char* HexChars = "0123456789abcdef";

	unsigned char HashedPassword[64 + 1];
	const char* PasswordString = "안녕";

	OpenSSL::EVP_MD_CTX* Ctx = OpenSSL::EVP_MD_CTX_new();
	check(Ctx);

	check(1 == EVP_DigestInit_ex(Ctx, OpenSSL::EVP_sha512(), nullptr));

	check(1 == EVP_DigestUpdate(Ctx, PasswordString, 64));

	check(1 == EVP_DigestFinal_ex(Ctx, HashedPassword, nullptr))
	HashedPassword[64] = '\0';

	EVP_MD_CTX_free(Ctx);

	char HexString[129];
	for (size_t i = 0; i < 64; ++i)
	{
		HexString[i * 2] = HexChars[(HashedPassword[i] >> 4) & 0x0F];
		HexString[i * 2 + 1] = HexChars[HashedPassword[i] & 0x0F];
	}
	HexString[128] = '\0';
	RatkiniaClient->LoginRequest(0, 
		"asdasdasdas", HexString);
}

void ALoginGameMode::OnParseMessageFailed(uint64_t context, RatkiniaProtocol::StcMessageType messagetType)
{
}

void ALoginGameMode::OnUnknownMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messagetType)
{
}

void ALoginGameMode::OnUnhandledMessageType(uint64_t context, RatkiniaProtocol::StcMessageType messagetType)
{
}

void ALoginGameMode::OnLoginResponse(uint64_t context, const bool successful, const std::string& failure_reason)
{
}
