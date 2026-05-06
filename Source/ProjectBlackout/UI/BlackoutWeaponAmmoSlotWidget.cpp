#include "UI/BlackoutWeaponAmmoSlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBlackoutWeaponAmmoSlotWidget::SetWeaponAmmoData(const FBlackoutWeaponAmmoSlotData& NewSlotData)
{
	const bool bWasEquipped = SlotData.bIsEquipped;

	SlotData = NewSlotData;
	ReceiveWeaponAmmoDataChanged(SlotData);

	if (bWasEquipped != SlotData.bIsEquipped)
	{
		ReceiveEquippedStateChanged(SlotData.bIsEquipped);
	}
	
	if (WeaponImage)
	{
		WeaponImage->SetBrushFromTexture(SlotData.WeaponIcon);
	}

	if (CurrentAmmoText)
	{
		CurrentAmmoText->SetText(FText::AsNumber(SlotData.CurrentAmmo));
	}

	if (ReserveAmmoText)
	{
		ReserveAmmoText->SetText(FText::AsNumber(SlotData.ReserveAmmo));
	}
}
