// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAttributeSet.h"

UCombatAttributeSet::UCombatAttributeSet()
{
	// 初期値の設定
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitPosture(0.0f);
	InitMaxPosture(100.0f);
}
