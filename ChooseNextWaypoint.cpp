// Fill out your copyright notice in the Description page of Project Settings.

#include "ChooseNextWaypoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrolRoute.h"
#include "Runtime/AIModule/Classes/AIController.h"

EBTNodeResult::Type UChooseNextWaypoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Get Patrol Points
	AActor* Owner = OwnerComp.GetAIOwner()->GetPawn();
	UPatrolRoute* PatrolRoute = Owner->FindComponentByClass<UPatrolRoute>();
	if (!ensure(PatrolRoute)) { return EBTNodeResult::Failed; }

	TArray<AActor*> PatrolPoints = PatrolRoute->GetPatrolPoints();
	if (!PatrolPoints.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("SWAG: Patrol route did not contain any patrol points"));
		return EBTNodeResult::Failed;
	}

	// Set next waypoint
	const auto BlackboardComp = OwnerComp.GetBlackboardComponent();
	const int32 Index = BlackboardComp->GetValueAsInt(IndexKey.SelectedKeyName);
	BlackboardComp->SetValueAsObject(NextWaypointKey.SelectedKeyName, PatrolPoints[Index]);

	// Cycle index
	const int32 NextIndex = (Index + 1) % PatrolPoints.Num();
	BlackboardComp->SetValueAsInt(IndexKey.SelectedKeyName, NextIndex);

	return EBTNodeResult::Succeeded;
}