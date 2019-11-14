#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

USTRUCT()
struct FSpawnPosition
{
	GENERATED_USTRUCT_BODY();

	FVector Location;
	float YawRotation;
	float Scale;
};

USTRUCT(BlueprintType)
struct FActorSpawnVals
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite)
		int32 MinSpawnAmount = 1;
	UPROPERTY(BlueprintReadWrite)
		int32 MaxSpawnAmount = 1;
	UPROPERTY(BlueprintReadWrite)
		float Radius = 500;
	UPROPERTY(BlueprintReadWrite)
		float MaxScale = 1;
	UPROPERTY(BlueprintReadWrite)
		float MinScale = 1;
};

class UHierarchicalInstancedStaticMeshComponent;
class UActorPool;

UCLASS()
class TESTINGGROUNDS_API ATile : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Level generation")
		void SpawnActors(TSubclassOf<AActor> ActorToSpawn, FActorSpawnVals SpawnVals);

	UFUNCTION(BlueprintCallable, Category = "Level generation")
		void SpawnAIPawns(TSubclassOf<APawn> PawnToSpawn, FActorSpawnVals SpawnVals);

	UFUNCTION(BlueprintCallable, Category = "Level generation")
		void SpawnFoliage(UHierarchicalInstancedStaticMeshComponent* HISMC, int32 InstanceAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Pool")
		void SetPool(UActorPool* VolumePool);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Level generation")
		FVector MinTileExtent;

	UPROPERTY(EditDefaultsOnly, Category = "Level generation")
		FVector MaxTileExtent;

	UPROPERTY(EditDefaultsOnly, Category = "Level generation")
		FVector NavMeshBoundsOffset;

private:
	bool IsLocationBlocked(FVector Location, float Radius) const;

	bool FindEmptyLocation(float Radius, FVector& EmptyLocation) const;

	template<class T>
	void RandomlyPlaceActor(TSubclassOf<T> ToSpawn, FSpawnPosition SpawnPosition);

	template<class T>
	void RandomlyPlaceActor(TSubclassOf<T> ToSpawn, FActorSpawnVals SpawnVals);

	void PlaceActor(TSubclassOf<AActor> ActorToSpawn, FSpawnPosition SpawnPosition);

	void PlaceActor(TSubclassOf<APawn> ActorToSpawn, FSpawnPosition SpawnPosition);

	void PositionNavMeshBoundsVolume(UActorPool* VolumePool);

	TArray<FSpawnPosition> GenerateSpawnPositions(int32 MinSpawnAmount = 1, int32 MaxSpawnAmount = 1, float MaxScale = 1, float MinScale = 1, float Radius = 500) const;

	UActorPool* VolumePool = nullptr;

	AActor* UsedNavMeshVolume = nullptr;
};