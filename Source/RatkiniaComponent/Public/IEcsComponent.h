// Copyright floweryclover @ SolarFlora 2025. All rights reserved.

#pragma once

#include "ComponentOrder.gen.h"

namespace RatkiniaProtocol
{
	class ComponentVariant;
}

#define ECSCOMPONENT(Name) static constexpr uint16 RuntimeOrder = static_cast<uint16>(::RatkiniaProtocol::EComponentOrder::Name);

class IEcsComponent
{
public:
	virtual ~IEcsComponent() = default;
	virtual void UpdateEcsComponent(const RatkiniaProtocol::ComponentVariant& ComponentVariant) {}
};
