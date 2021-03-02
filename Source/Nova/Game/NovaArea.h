// Nova project - Gwennaël Arbona

#pragma once

#include "EngineMinimal.h"
#include "NovaGameTypes.h"
#include "NovaArea.generated.h"

/*----------------------------------------------------
    Description types
----------------------------------------------------*/

/** World area description */
UCLASS(ClassGroup = (Nova))
class UNovaArea : public UNovaAssetDescription
{
	GENERATED_BODY()

public:
	// Sub-level to load
	UPROPERTY(Category = Nova, EditDefaultsOnly)
	FName LevelName;

	// Altitude in kilometers
	UPROPERTY(Category = Nova, EditDefaultsOnly)
	float Altitude;

	// Initial phase on the orbit in degrees
	UPROPERTY(Category = Nova, EditDefaultsOnly)
	float Phase;
};
