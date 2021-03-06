#include "hud.hpp"

extern IEngine *engine;
extern IEntities *ents;

extern ISurface *surface;

extern Globals *globals;


extern int m_angRotation;
extern int m_Collision;
extern int m_vecMins, m_vecMaxs;
extern int m_mobRush;

extern int m_nSolidType, m_usSolidFlags;

bool hud::MakeBox(Entity *ent, int &x0, int &y0, int &x1, int &y1)
{
	const Vector edges[4] =
	{
		Vector(1.0f , 1.0f, 1.0f),
		Vector(-1.0f, 1.0f, 1.0f),
		Vector(1.0f ,-1.0f, 1.0f),
		Vector(-1.0f,-1.0f, 1.0f),
	};

	Vector vec = ent->GetPos();
	Angle  ang = ReadPtr<Angle>(ent, m_angRotation);

	Vector mins = ReadPtr<Vector>(ent, m_Collision + m_vecMins);
	Vector maxs = ReadPtr<Vector>(ent, m_Collision + m_vecMaxs);

	x0 = maxof<int>();
	y0 = maxof<int>();
	x1 = minof<int>();
	y1 = minof<int>();

	for (int i = 0; i < 4; i++)
	{
		Vector mins2d, maxs2d;

		if ((vec + (mins * edges[i].Rotate(ang))).ToScreen(mins2d) == false)
			return false;

		if ((vec + (maxs * edges[i].Rotate(ang))).ToScreen(maxs2d) == false)
			return false;

		x0 = Min<int>(x0, Min(mins2d.x, maxs2d.x));
		y0 = Min<int>(y0, Min(mins2d.y, maxs2d.y));
		x1 = Max<int>(x1, Max(mins2d.x, maxs2d.x));
		y1 = Max<int>(y1, Max(mins2d.y, maxs2d.y));
	}

	return true;
}

void hud::DrawESP()
{
	surface->SetFont(ui::font_hud);
	surface->SetTextColor(0xff, 0xff, 0xff, 0xff);

	Entity *lp  = LocalPlayer();
	int  maxent = globals->maxclients();

	for (int i = 1; i <= ents->GetMaxEntities(); ++i)
	{
		Entity *ent = ents->GetEntity(i);

		if (ent == nullptr || ent == lp)
			continue;

		if (ent->IsDormant())
			continue;

		if (1 && i <= maxent)
		{
			if (!ent->IsAlive())
				continue;

			int x0, y0, x1, y1;

			if (!MakeBox(ent, x0, y0, x1, y1))
			 	continue;

			surface->SetColor(0x00, 0x00, 0x00, 0x7f);
			surface->DrawOutlinedRect(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
			surface->DrawOutlinedRect(x0 + 1, y0 + 1, x1 - 1, y1 - 1);

			if (1) // menu.ESP.health
			 	surface->DrawFilledRect(x0 - 4, y0 - 1, x0 - 1, y1 + 1);

#if defined(L4D) || defined(L4D2)
			switch (ent->GetTeam())
			{
				case 2:
				case 4:
					surface->SetColor(0x46, 0x78, 0xff, 0xff);
				break;

				case 3:
					surface->SetColor(0x96, 0x00, 0xe1, 0xff);
				break;
			}
#else
			switch (ent->GetTeam())
			{
				case TEAM_RED:
					surface->SetColor(0xff, 0x64, 0x64, 0xff);
				break;

				case TEAM_BLU:
					surface->SetColor(0x46, 0x78, 0xff, 0xff);
				break;
			}
#endif

			surface->DrawOutlinedRect(x0, y0, x1, y1);

			if (1) // menu.ESP.health
			{
				if (ent->GetHealth() <= ent->GetMaxHealth())
				{
					if (ent->GetHealth() <= (ent->GetMaxHealth() / 4))
						surface->SetColor(0xff, 0x00, 0x00, 0xff);
					else
						surface->SetColor(0x00, 0xff, 0x00, 0xff);
				}

				surface->DrawFilledRect(x0 - 3, y1 - Min<int>(y1 - y0, (float)(y1 - y0) * ((float)ent->GetHealth() / (float)ent->GetMaxHealth()) + 0.5f), x0 - 1, y1);
			}

			if (1) // menu.ESP.name
			{
				player_info info;
				engine->GetPlayerInfo(i, info);

				int length = 0;
				const wchar_t *text = tounicode(info.name, length);

				int tw, th;
				surface->GetTextSize(ui::font_hud, text, tw, th);

				surface->SetTextPos((x0 + x1 - tw) / 2, y0 - th);
				surface->DrawText(text, length, 0);
			}

			if (1) // menu.ESP.weapon
			{
				Entity *weapon = ent->GetActiveWeapon();

				if (weapon)
				{
					const char *classname = weapon->GetClass();
#ifndef GMOD
					if (qstrlen(classname) > 6)
					{
						classname += 6;
					}
#endif

					int length = 0;
					const wchar_t *text = util::MakeReadable(classname, length);

					int tw, th;
					surface->GetTextSize(ui::font_hud, text, tw, th);

					surface->SetTextPos((x0 + x1 - tw) / 2, y1 + 1);
					surface->DrawText(text, length, 0);
				}
			}

			continue;
		}

		if (1)
		{
#if defined(L4D) || defined(L4D2)
			if (streq(ent->GetNetworkClass()->name, "Infected"))
			{
				if (!ent->NPC_IsAlive())
					continue;

				int x0, y0, x1, y1;

				if (!MakeBox(ent, x0, y0, x1, y1))
					continue;

				surface->SetColor(0x00, 0x00, 0x00, 0x7f);
				surface->DrawOutlinedRect(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
				surface->DrawOutlinedRect(x0 + 1, y0 + 1, x1 - 1, y1 - 1);

				surface->SetColor(0xc8, 0xff, 0x00, 0xff);
				surface->DrawOutlinedRect(x0, y0, x1, y1);


				int tw, th;

				if (ReadPtr<bool>(ent, m_mobRush))
				{
					surface->GetTextSize(ui::font_hud, L"Mob rush", tw, th);

					surface->SetTextPos((x0 + x1 - tw) / 2, y1 + 1);
					surface->DrawText(L"Mob rush", 8, 0);
				}
				else
				{
					surface->GetTextSize(ui::font_hud, L"Common", tw, th);

					surface->SetTextPos((x0 + x1 - tw) / 2, y1 + 1);
					surface->DrawText(L"Common", 6, 0);
				}

				continue;
			}

			if (streq(ent->GetNetworkClass()->name, "Witch"))
			{
				if (!ent->NPC_IsAlive())
					continue;

				int x0, y0, x1, y1;

				if (!MakeBox(ent, x0, y0, x1, y1))
					continue;

				surface->SetColor(0x00, 0x00, 0x00, 0x7f);
				surface->DrawOutlinedRect(x0 - 1, y0 - 1, x1 + 1, y1 + 1);
				surface->DrawOutlinedRect(x0 + 1, y0 + 1, x1 - 1, y1 - 1);

				surface->SetColor(0x96, 0x00, 0xe1, 0xff);
				surface->DrawOutlinedRect(x0, y0, x1, y1);

				int tw, th;

				surface->GetTextSize(ui::font_hud, L"Witch", tw, th);

				surface->SetTextPos((x0 + x1 - tw) / 2, y1 + 1);
				surface->DrawText(L"Witch", 5, 0);

				continue;
			}
#endif
		}
	}
}

void hud::DrawCrosshair()
{
	int x, y;
	engine->GetScreenSize(x, y);

	x /= 2;
	y /= 2;

	surface->SetColor(0xff, 0xff, 0xff, 0xff);
	surface->DrawFilledRect(x - 2, y - 1, x, y + 1);
	surface->DrawFilledRect(x, y - 1, x + 2, y + 1);
	surface->DrawFilledRect(x - 1, y - 2, x + 1, y);
	surface->DrawFilledRect(x - 1, y, x + 1, y + 2);
}