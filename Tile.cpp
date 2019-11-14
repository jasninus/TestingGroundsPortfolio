#include "Tile.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "EngineUtils.h"
#include "ActorPool.h"
#include "AI/Navigation/NavigationSystem.h"

// Sets default values
ATile::ATile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// These values will show up as default values in Blueprint. Blueprint values will override!
	MinTileExtent = FVector(0, -1950, 100);
	MaxTileExtent = FVector(3950, 1950, 100);
	NavMeshBoundsOffset = FVector(2000, 0, 0);
}

// Called when the game starts or when spawned
void ATile::BeginPlay()
{
	Super::BeginPlay();
}

void ATile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!UsedNavMeshVolume || !VolumePool)
	{
		return;
	}

	VolumePool->Return(UsedNavMeshVolume);
}

// Called every frame
void ATile::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATile::SpawnActors(const TSubclassOf<AActor> ActorToSpawn, const FActorSpawnVals SpawnVals)
{
	RandomlyPlaceActor(ActorToSpawn, SpawnVals);
}

void ATile::SpawnAIPawns(const TSubclassOf<APawn> PawnToSpawn, const FActorSpawnVals SpawnVals)
{
	RandomlyPlaceActor(PawnToSpawn, SpawnVals);
}

template <class T>
void ATile::RandomlyPlaceActor(TSubclassOf<T> ToSpawn, const FActorSpawnVals SpawnVals)
{
	const int32 SpawnAmount = FMath::RandRange(SpawnVals.MinSpawnAmount, SpawnVals.MaxSpawnAmount);

	for (int i = 0; i < SpawnAmount; ++i)
	{
		FSpawnPosition NewSpawnPos;

		NewSpawnPos.Scale = FMath::FRandRange(SpawnVals.MinScale, SpawnVals.MaxScale);

		if (!FindEmptyLocation(SpawnVals.Radius * NewSpawnPos.Scale, NewSpawnPos.Location))
		{
			UE_LOG(LogTemp, Warning, TEXT("Spawn position for actor could not be found"));
			continue; // Don't add if no locations can be found
		}

		NewSpawnPos.YawRotation = FMath::FRandRange(-180, 180);

		PlaceActor(ToSpawn, NewSpawnPos);
	}
}

void ATile::PlaceActor(const TSubclassOf<AActor> ActorToSpawn, const FSpawnPosition SpawnPosition)
{
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorToSpawn);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SWAG: Spawned null actor"));
		return;
	}

	SpawnedActor->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedActor->SetActorRelativeLocation(SpawnPosition.Location);
	SpawnedActor->SetActorRotation(FRotator(0, SpawnPosition.YawRotation, 0));
	SpawnedActor->SetActorScale3D(FVector(SpawnPosition.Scale));
}

void ATile::PlaceActor(const TSubclassOf<APawn> ActorToSpawn, const FSpawnPosition SpawnPosition)
{
	// FVector const& Location, FRotator const& Rotation, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters()

	FActorSpawnParameters SpawnParameters = FActorSpawnParameters();

	APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(ActorToSpawn, FVector(0, 0, 1000), FRotator(0), SpawnParameters);

	if (!SpawnedPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("SWAG: Spawned null pawn"));
		return;
	}

	SpawnedPawn->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	SpawnedPawn->SetActorRelativeLocation(SpawnPosition.Location);
	SpawnedPawn->SetActorRotation(FRotator(0, SpawnPosition.YawRotation, 0));
	SpawnedPawn->SpawnDefaultController();
}

// Returns true if empty location could be found
bool ATile::FindEmptyLocation(const float Radius, FVector& EmptyLocation) const
{
	for (size_t i = 0; i < 100; i++)
	{
		const FVector LocationToTest = FMath::RandPointInBox(FBox(
			MinTileExtent,
			MaxTileExtent
		));

		if (!IsLocationBlocked(LocationToTest, Radius))
		{
			EmptyLocation = LocationToTest;
			return true;
		}
	}

	return false;
}

bool ATile::IsLocationBlocked(const FVector Location, const float Radius) const
{
	const FVector GlobalLocation = ActorToWorld().TransformPosition(Location);

	FHitResult HitResult;
	const bool HasHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		GlobalLocation,
		GlobalLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeSphere(Radius)
	);

	return HasHit;
}

void ATile::SpawnFoliage(UHierarchicalInstancedStaticMeshComponent* HISMC, const int32 InstanceAmount) const
{
	const FVector GrassMinExtent = MinTileExtent - FVector(0, 0, 100);
	const FVector GrassMaxExtent = MaxTileExtent - FVector(0, 0, 100);

	for (size_t i = 0; i < InstanceAmount; i++)
	{
		const FQuat RandRot = FVector(0, FMath::FRandRange(-180, 180), 0).Rotation().Quaternion();
		const FVector RandPos = FMath::RandPointInBox(FBox(
			GrassMinExtent,
			GrassMaxExtent
		));

		HISMC->AddInstance(FTransform(RandRot, RandPos));
	}
}

void ATile::SetPool(UActorPool* VolumePool)
{
	this->VolumePool = VolumePool;

	PositionNavMeshBoundsVolume(VolumePool);
}

void ATile::PositionNavMeshBoundsVolume(UActorPool* VolumePool)
{
	UsedNavMeshVolume = VolumePool->CheckOut();

	// Set nav mesh volume from pool to have same position as tile
	if (!UsedNavMeshVolume)
	{
		UE_LOG(LogTemp, Error, TEXT("SWAG: Not enough actors in NavMeshBoundsVolume pool"));
		return;
	}

	UsedNavMeshVolume->SetActorLocation(GetActorLocation() + NavMeshBoundsOffset);
	GetWorld()->GetNavigationSystem()->Build(); // Rebuild all NavMeshBoundsVolumes
}