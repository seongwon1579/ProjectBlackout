#include "UI/BlackoutWeaponAmmoWidget.h"

#include "UI/BlackoutWeaponAmmoSlotWidget.h"

void UBlackoutWeaponAmmoWidget::SetWeaponAmmoData(const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
	const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData, bool bPlaySwapAnimation)
{
	PrimarySlotData = PrimaryWeaponData;
	SecondarySlotData = SecondaryWeaponData;
	
	if (PrimaryWeaponSlotWidget)
	{
		PrimaryWeaponSlotWidget->SetWeaponAmmoData(PrimarySlotData);
	}
	
	if (SecondaryWeaponSlotWidget)
	{
		SecondaryWeaponSlotWidget->SetWeaponAmmoData(SecondarySlotData);
	}
	
	ReceiveWeaponAmmoDisplayChanged(PrimarySlotData, SecondarySlotData);

	if (bPlaySwapAnimation)
	{
		ReceiveWeaponSwapAnimationRequested(PrimarySlotData.bIsEquipped);
	}
}
