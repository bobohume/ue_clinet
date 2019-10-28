#pragma once  

#include "Engine/DataTable.h"  
#include "CoreMinimal.h"
#include "MapData.generated.h"  

USTRUCT(BlueprintType)
struct FMapData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FMapData()
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MapData)
	int32 Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MapData)
	int32 Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MapData)
	int32 Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MapData)
	int32 Height;
};