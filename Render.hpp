#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>
#include <map>
#include "Entity.h"

namespace Render
{
	// �������
	ImVec4 Get2DBox(const CEntity& Entity)
	{
		BoneJointPos Head = Entity.GetBone().BonePosList[BONEINDEX::head];

		Vec2 Size, Pos;
		Size.y = (Entity.Pawn.ScreenPos.y - Head.ScreenPos.y) * 1.09;
		Size.x = Size.y * 0.6;

		Pos = ImVec2(Entity.Pawn.ScreenPos.x - Size.x / 2, Head.ScreenPos.y- Size.y*0.08);

		return ImVec4{ Pos.x,Pos.y,Size.x,Size.y };
	}

	// ��������
	void DrawBone(const CEntity& Entity, ImColor Color, float Thickness)
	{
		BoneJointPos Previous, Current;
		for (auto i : BoneJointList::List)
		{
			Previous.Pos = Vec3(0, 0, 0);
			for (auto Index : i)
			{
				Current = Entity.GetBone().BonePosList[Index];
				if (Previous.Pos == Vec3(0, 0, 0))
				{
					Previous = Current;
					//pGame->View->Gui->Text(Current.Name.c_str(), Current.ScreenPos, ImColor(255, 255, 0, 255));
					continue;
				}
				if (Previous.IsVisible && Current.IsVisible)
				{
					Gui.Line(Previous.ScreenPos, Current.ScreenPos, Color, Thickness);
					//pGame->View->Gui->Text(Current.Name.c_str(), Current.ScreenPos, ImColor(255, 255, 0, 255));
				}
				Previous = Current;
			}
		}
	}

	// �������
	void ShowLosLine(const CEntity& Entity, const float Length, ImColor Color, float Thickness)
	{
		Vec2 StartPoint, EndPoint;
		Vec3 Temp;
		BoneJointPos Head = Entity.GetBone().BonePosList[BONEINDEX::head];

		StartPoint = Head.ScreenPos;

		float LineLength = cos(Entity.Pawn.ViewAngle.x * M_PI / 180) * Length;

		Temp.x = Head.Pos.x + cos(Entity.Pawn.ViewAngle.y * M_PI / 180) * LineLength;
		Temp.y = Head.Pos.y + sin(Entity.Pawn.ViewAngle.y * M_PI / 180) * LineLength;
		Temp.z = Head.Pos.z - sin(Entity.Pawn.ViewAngle.x * M_PI / 180) * Length;

		if (!gGame.View.WorldToScreen(Temp, EndPoint))
			return;

		Gui.Line(StartPoint, EndPoint, Color, Thickness);
	}

	// 2D���������
	ImVec4 Get2DBoneRect(const CEntity& Entity)
	{
		Vec2 Min, Max, Size;
		Min = Max = Entity.GetBone().BonePosList[0].ScreenPos;

		for (auto &BoneJoint : Entity.GetBone().BonePosList)
		{
			if (!BoneJoint.IsVisible)
				continue;
			Min.x = min(BoneJoint.ScreenPos.x, Min.x);
			Min.y = min(BoneJoint.ScreenPos.y, Min.y);
			Max.x = max(BoneJoint.ScreenPos.x, Max.x);
			Max.y = max(BoneJoint.ScreenPos.y, Max.y);
		}
		Size.x = Max.x - Min.x;
		Size.y = Max.y - Min.y;

		return ImVec4(Min.x, Min.y, Size.x, Size.y);
	}

	class HealthBar
	{
	private:
		using TimePoint_ = std::chrono::steady_clock::time_point;
	private:
		// ��ʾ����Ѫ��ʱ��(ms)
		const int ShowBackUpHealthDuration = 450;
		// ���Ѫ��
		float MaxHealth = 0.f;
		// ��ǰѪ��
		float CurrentHealth = 0.f;
		// �������Ѫ����С
		float LastestBackupHealth = 0.f;
		// Ѫ������
		ImVec2 RectPos{};
		// Ѫ����С
		ImVec2 RectSize{};
		// ������ʾ����Ѫ��
		bool InShowBackupHealth = false;
		// ��ʾ����Ѫ����ʼʱ���
		TimePoint_ BackupHealthTimePoint{};
	public:
		// ��ʼ��
		HealthBar(float MaxHealth_) :MaxHealth(MaxHealth_) {}
		HealthBar() { MaxHealth = 100; }
		// ����Ѫ��
		void DrawHealthBar(float CurrentHealth, ImVec2 Pos, ImVec2 Size);
	private:
		// ��һ�׶�Ѫ����ɫ 0.5-1
		ImColor FirstStageColor = ImColor(96, 246, 113, 220);
		// �ڶ��׶�Ѫ����ɫ 0.5-0.2
		ImColor SecondStageColor = ImColor(247, 214, 103, 220);
		// �����׶�Ѫ����ɫ 0.2-0.0
		ImColor ThirdStageColor = ImColor(255, 95, 95, 220);
		// ����Ѫ����ɫ
		ImColor BackupHealthColor = ImColor(255, 255, 255, 220);
		// �߿���ɫ
		ImColor FrameColor = ImColor(45, 45, 45, 220);
		// ������ɫ
		ImColor BackGroundColor = ImColor(90, 90, 90, 220);
	};

	void HealthBar::DrawHealthBar(float CurrentHealth, ImVec2 Pos, ImVec2 Size)
	{
		auto InRange = [&](float value, float min, float max) -> bool
		{
			return value > min && value <= max;
		};

		ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

		this->CurrentHealth = CurrentHealth;
		this->RectPos = Pos;
		this->RectSize = Size;

		// ռ��
		float Proportion = CurrentHealth / MaxHealth;
		// Ѫ�������
		float Width = RectSize.x * Proportion;
		// Ѫ������ɫ
		ImColor Color;

		// ����
		DrawList->AddRectFilled(RectPos,
			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },
			BackGroundColor, 5, 15);

		// ��ɫ�л�
		if (InRange(Proportion, 0.5, 1))
			Color = FirstStageColor;
		else if (InRange(Proportion, 0.2, 0.5))
			Color = SecondStageColor;
		else if (InRange(Proportion, 0, 0.2))
			Color = ThirdStageColor;

		// �����������Ѫ��
		if (LastestBackupHealth == 0
			|| LastestBackupHealth < CurrentHealth)
			LastestBackupHealth = CurrentHealth;

		if (LastestBackupHealth != CurrentHealth)
		{
			if (!InShowBackupHealth)
			{
				BackupHealthTimePoint = std::chrono::steady_clock::now();
				InShowBackupHealth = true;
			}

			std::chrono::steady_clock::time_point CurrentTime = std::chrono::steady_clock::now();
			if (CurrentTime - BackupHealthTimePoint > std::chrono::milliseconds(ShowBackUpHealthDuration))
			{
				// ��ʱ��ֹͣ��ʾ����Ѫ�������Ҹ����������Ѫ��
				LastestBackupHealth = CurrentHealth;
				InShowBackupHealth = false;
			}

			if (InShowBackupHealth)
			{
				// ����Ѫ�����ƿ��
				float BackupHealthWidth = LastestBackupHealth / MaxHealth * RectSize.x;
				// ����Ѫ��alpha����
				float BackupHealthColorAlpha = 1 - 0.95 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);
				ImColor BackupHealthColorTemp = BackupHealthColor;
				BackupHealthColorTemp.Value.w = BackupHealthColorAlpha;
				// ����Ѫ����Ȼ���
				float BackupHealthWidth_Lerp = 0.7 * (std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - BackupHealthTimePoint).count() / (float)ShowBackUpHealthDuration);
				BackupHealthWidth_Lerp *= (BackupHealthWidth - Width);
				BackupHealthWidth -= BackupHealthWidth_Lerp;
				// ����Ѫ��
				DrawList->AddRectFilled(RectPos,
					{ RectPos.x + BackupHealthWidth,RectPos.y + RectSize.y },
					BackupHealthColorTemp, 5);
			}
		}

		// Ѫ��
		DrawList->AddRectFilled(RectPos,
			{ RectPos.x + Width,RectPos.y + RectSize.y },
			Color, 5);

		// �߿�
		DrawList->AddRect(RectPos,
			{ RectPos.x + RectSize.x,RectPos.y + RectSize.y },
			FrameColor, 5, 15, 2);
	}

}