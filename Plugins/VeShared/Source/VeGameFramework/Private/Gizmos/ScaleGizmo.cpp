﻿// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.
// Copyright (c) 2022-2023 VeVerse AS.
// Copyright (c) 2023 LE7EL AS. All rights reserved.


#include "Gizmos/ScaleGizmo.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

AScaleGizmo::AScaleGizmo() {
	XY_PlaneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("XY Plane"));
	YZ_PlaneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("YZ Plane"));
	XZ_PlaneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("XZ Plane"));
	XYZ_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("XYZ Sphere"));

	XY_PlaneBox->SetupAttachment(ScalingSceneComponent);
	YZ_PlaneBox->SetupAttachment(ScalingSceneComponent);
	XZ_PlaneBox->SetupAttachment(ScalingSceneComponent);
	XYZ_Sphere->SetupAttachment(ScalingSceneComponent);

	RegisterDomainComponent(XY_PlaneBox, ETransformationDomain::PlaneXY);
	RegisterDomainComponent(YZ_PlaneBox, ETransformationDomain::PlaneYZ);
	RegisterDomainComponent(XZ_PlaneBox, ETransformationDomain::PlaneXZ);
	RegisterDomainComponent(XYZ_Sphere, ETransformationDomain::XYZ);

	ScalingFactor = 0.05f;

}

void AScaleGizmo::UpdateGizmoSpace(ETransformSpace SpaceType) {
	//Force to always be Local
	SetActorRelativeRotation(FQuat(EForceInit::ForceInit));
}

FTransform AScaleGizmo::GetDeltaTransform(const FVector& LookingVector, const FVector& RayStartPoint, const FVector& RayEndPoint, ETransformationDomain Domain) {
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector);

	if (AreRaysValid()) {
		const float Cos45Deg = 0.707;

		// the opposite direction of the Normal that is most perpendicular to the Looking Vector
		// will be the one we choose to be the normal to the Domain! (needs to be calculated for axis. For planes, it's straightforward)
		FVector planeNormal;

		FVector targetDirection(0.f);

		//we have two vectors for each direction, one world and one local
		// world is to calculate the scale direction (e.g. for X scale it would have to be multiplied by 1.f, 0.f, 0.f)
		// local is to calculate the direction of the mouse and calculate how much it moved!

		FVector forwardVector = GetActorForwardVector();
		FVector rightVector = GetActorRightVector();
		FVector upVector = GetActorUpVector();;


		switch (Domain) {
		case ETransformationDomain::AxisX: {
			targetDirection = forwardVector;
			if (FMath::Abs(FVector::DotProduct(LookingVector, rightVector)) > Cos45Deg) planeNormal = rightVector;
			else planeNormal = upVector;
			break;
		}
		case ETransformationDomain::AxisY: {
			targetDirection = rightVector;
			if (FMath::Abs(FVector::DotProduct(LookingVector, forwardVector)) > Cos45Deg) planeNormal = forwardVector;
			else planeNormal = upVector;
			break;
		}
		case ETransformationDomain::AxisZ: {
			targetDirection = upVector;
			if (FMath::Abs(FVector::DotProduct(LookingVector, forwardVector)) > Cos45Deg) planeNormal = forwardVector;
			else planeNormal = rightVector;
			break;
		}
		case ETransformationDomain::PlaneXY:
			targetDirection = forwardVector + rightVector;
			planeNormal = upVector;
			break;
		case ETransformationDomain::PlaneYZ:
			targetDirection = rightVector + upVector;
			planeNormal = forwardVector;
			break;
		case ETransformationDomain::PlaneXZ:
			targetDirection = forwardVector + upVector;
			planeNormal = rightVector;
			break;
		case ETransformationDomain::XYZ:
			targetDirection = forwardVector + rightVector + upVector;
			planeNormal = LookingVector;
			break;
		}

		FPlane plane;
		plane.X = planeNormal.X;
		plane.Y = planeNormal.Y;
		plane.Z = planeNormal.Z;
		plane.W = FVector::PointPlaneDist(GetActorLocation(), FVector::ZeroVector, planeNormal);

		FVector deltaLocation =
			FMath::LinePlaneIntersection(RayStartPoint, RayEndPoint, plane)
			- FMath::LinePlaneIntersection(PreviousRayStartPoint, PreviousRayEndPoint, plane);

		deltaLocation = deltaLocation.ProjectOnTo(targetDirection);
		deltaTransform.SetScale3D(deltaLocation * ScalingFactor);

	}

	UpdateRays(RayStartPoint, RayEndPoint);

	return deltaTransform;
}

FTransform AScaleGizmo::GetSnappedTransform(FTransform& outCurrentAccumulatedTransform, const FTransform& DeltaTransform, ETransformationDomain Domain, float SnappingValue) const {
	if (SnappingValue == 0.f) return DeltaTransform;

	FTransform result = DeltaTransform;
	FVector addedScale = outCurrentAccumulatedTransform.GetScale3D() + DeltaTransform.GetScale3D();

	float domains = 1.f;

	switch (Domain) {
	case ETransformationDomain::PlaneXY:
	case ETransformationDomain::PlaneYZ:
	case ETransformationDomain::PlaneXZ:
		domains = 2.f;
		break;
	case ETransformationDomain::XYZ:
		domains = 3.f;
		break;
	}

	FVector snappedScale = addedScale.GetSafeNormal()
		* FMath::GridSnap(addedScale.Size(), FMath::Sqrt(FMath::Square(SnappingValue) * domains));

	result.SetScale3D(snappedScale);
	outCurrentAccumulatedTransform.SetScale3D(addedScale - snappedScale);
	return result;
}

FTransform AScaleGizmo::GetSnappedTransformPerComponent(const FTransform& OldComponentTransform, const FTransform& NewComponentTransform, ETransformationDomain Domain, float SnappingValue) const {

	FTransform result = NewComponentTransform;

	FVector newScale = NewComponentTransform.GetScale3D();
	if (!newScale.Equals(OldComponentTransform.GetScale3D(), 0.0001f)) {
		FVector domainScale;
		switch (Domain) {
		case ETransformationDomain::AxisX:
			domainScale = FVector(1.f, 0.f, 0.f);
			break;
		case ETransformationDomain::AxisY:
			domainScale = FVector(0.f, 1.f, 0.f);
			break;
		case ETransformationDomain::AxisZ:
			domainScale = FVector(0.f, 0.f, 1.f);
			break;

		case ETransformationDomain::PlaneXY:
			domainScale = FVector(1.f, 1.f, 0.f);
			break;
		case ETransformationDomain::PlaneYZ:
			domainScale = FVector(0.f, 1.f, 1.f);
			break;
		case ETransformationDomain::PlaneXZ:
			domainScale = FVector(1.f, 0.f, 1.f);
			break;

		case ETransformationDomain::XYZ:
			domainScale = FVector(1.f, 1.f, 1.f);
			break;
		}
		FVector domainInverseScale = FVector::OneVector - domainScale;

		newScale = newScale.GridSnap(SnappingValue);
		FVector scale = (newScale * domainScale) + (NewComponentTransform.GetScale3D() * domainInverseScale);
		result.SetScale3D(scale);
	}

	return result;
}
