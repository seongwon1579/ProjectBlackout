// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutClassSelectWidget.h"
#include "UI/BlackoutClassSelectWidgetController.h"

void UBlackoutClassSelectWidget::SetWidgetController(
	UBlackoutClassSelectWidgetController* InController)
{
	UnbindWidgetControllerCallbacks();
	
	WidgetController = InController;
	if (!WidgetController)
	{
		return;
	}
	WidgetController->OnSelectionChanged.AddDynamic(this, &UBlackoutClassSelectWidget::HandleSelectionChanged);
	WidgetController->OnSelectionConfirmed.AddDynamic(this, &UBlackoutClassSelectWidget::HandleSelectionConfirmed);
	
	WidgetController->BroadcastCurrentSelection();
}

void UBlackoutClassSelectWidget::RequestNavigateNext()
{
	if (WidgetController)
	{
		WidgetController->NavigateNext();
	}
}

void UBlackoutClassSelectWidget::RequestNavigatePrevious()
{
	if (WidgetController)
	{
		WidgetController->NavigatePrevious();
	}
}

void UBlackoutClassSelectWidget::RequestConfirm()
{
	if (WidgetController)
	{
		WidgetController->ConfirmSelection();
	}
}

void UBlackoutClassSelectWidget::NativeDestruct()
{
	UnbindWidgetControllerCallbacks();
	Super::NativeDestruct();
}

void UBlackoutClassSelectWidget::HandleSelectionChanged(
	const FBlackoutClassSelectDisplayData& DisplayData)
{
	ReceiveSelectChanged(DisplayData);
}

void UBlackoutClassSelectWidget::HandleSelectionConfirmed()
{
	ReceiveSelectConfirmed();
}

void UBlackoutClassSelectWidget::UnbindWidgetControllerCallbacks()
{
	if (!WidgetController)
	{
		return;
	}
	
	WidgetController->OnSelectionChanged.RemoveDynamic(this, &UBlackoutClassSelectWidget::HandleSelectionChanged);
	WidgetController->OnSelectionConfirmed.RemoveDynamic(this, &UBlackoutClassSelectWidget::HandleSelectionConfirmed);
}
