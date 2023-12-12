﻿// Copyright 2020 Juan Marcelo Portillo. All Rights Reserved.
// Copyright (c) 2022-2023 VeVerse AS.
// Copyright (c) 2023 LE7EL AS. All rights reserved.

#include "Gizmos/TranslationGizmo.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

ATranslationGizmo::ATranslationGizmo() {
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

}

FTransform ATranslationGizmo::GetDeltaTransform(const FVector& LookingVector, const FVector& RayStartPoint, const FVector& RayEndPoint, ETransformationDomain Domain) {
	FTransform deltaTransform;
	deltaTransform.SetScale3D(FVector::ZeroVector); //used so that the default FVector(1.f, 1.f, 1.f) does not affect further scaling

	if (AreRaysValid()) {
		const float Cos45Deg = 0.707;

		// the opposite direction of the Normal that is most perpendicular to the Looking Vector
		// will be the one we choose to be the normal to the Domain! (needs to be calculated for axis. For planes, it's straightforward)
		FVector planeNormal;

		// the direction of travel (only used for Axis Domains)
		FVector targetDirection(0.f);

		FVector forwardVector = GetActorForwardVector();
		FVector rightVector = GetActorRightVector();
		FVector upVector = GetActorUpVector();


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
			planeNormal = upVector;
			break;
		case ETransformationDomain::PlaneYZ:
			planeNormal = forwardVector;
			break;
		case ETransformationDomain::PlaneXZ:
			planeNormal = rightVector;
			break;
		case ETransformationDomain::XYZ:
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

		switch (Domain) {
		case ETransformationDomain::AxisX:
		case ETransformationDomain::AxisY:
		case ETransformationDomain::AxisZ:
			deltaLocation = deltaLocation.ProjectOnTo(targetDirection);
			break;
		}

		deltaTransform.SetLocation(deltaLocation);

	}

	UpdateRays(RayStartPoint, RayEndPoint);

	return deltaTransform;
}

FTransform ATranslationGizmo::GetSnappedTransform(FTransform& outCurrentAccumulatedTransform, const FTransform& DeltaTransform, ETransformationDomain Domain, float SnappingValue) const {
	if (SnappingValue == 0.f) return DeltaTransform;

	FTransform result = DeltaTransform;
	FVector addedLocation = outCurrentAccumulatedTransform.GetLocation() + DeltaTransform.GetLocation();

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

	FVector snappedLocation = addedLocation.GetSafeNormal()
		* FMath::GridSnap(addedLocation.Size(), FMath::Sqrt(FMath::Square(SnappingValue) * domains));

	result.SetLocation(snappedLocation);
	outCurrentAccumulatedTransform.SetLocation(addedLocation - snappedLocation);
	return result;
}