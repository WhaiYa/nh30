#include "main.h"

static bool speedfix[0x1000] = {0};


IClient		*client;
IEngine		*engine;
IEntities	*ents;
ITrace 		*enginetrace;
IModelInfo	*mdlinfo;
IMovement	*movement;
IPrediction	*prediction;
IEngineVGui *enginevgui;
ISurface	*surface;

Globals		*globals;


void (__thiscall *org_Paint)(void *, int);
void __fastcall hooked_Paint(void *t, void *, int paintmode)
{
	org_Paint(t, paintmode);

	if (paintmode & 2)
	{
		if (engine->IsInGame())
		{
			hud::DrawESP();
			hud::DrawCrosshair();
		}

		//
	}
}

void (__thiscall *org_SetViewAngles)(void *, Angle &);
void __fastcall hooked_SetViewAngles(void *t, void *, Angle &angles)
{
	bpaware();

	register UserCmd *ucmd asm("esi");

#if defined(GMOD)
	if (ucmd && ucmd->tick_count == 0 && ucmd->predicted)
		return;
#endif

	org_SetViewAngles(t, angles);


	if (ucmd && ucmd->command_number == bp->next->arg<int>(1))
	{
		Entity *lp = LocalPlayer();
		Entity *weapon = lp->GetActiveWeapon();

		float curtime = (float)lp->GetTickCount() * globals->interval();

		if (!lp->IsAlive())
			return;


		static int speedcmd = 0;

		if (speedfix[(ucmd->command_number+1) & (sizeof(speedfix)-1)] = (GetAsyncKeyState(VK_LSHIFT) && --speedcmd > 0))
		{
			bp->next->next->next->next->ret -= 5;
		}
		else
		{
			speedcmd = 5;
		}


		aimbot::RunCommand(ucmd);

		Angle viewang = ucmd->viewangles;
		bool canshoot = weapon && weapon->Clip1() > 0;

#if defined(VORANGEBOX)
		bool &sendpacket = *((bool *)bp->next->next->next - 1);
#endif
#if defined(VL4D)
		bool &sendpacket = *((bool *)bp->next->next->next - 0x21);
#endif
#if defined(CSGO)
		bool &sendpacket = *((bool *)bp->next - 0x1c);
#endif

		if (1 && !lp->HasFlag(FL_ONGROUND)) // menu/bhop
			ucmd->buttons.del(IN_JUMP);

		if (1 && canshoot) // menu.aimbot
		{
			if (aimbot::Think(ucmd) && 1)
			{
				org_SetViewAngles(t, ucmd->viewangles);
			}
		}

		if (1) // menu.norecoil
		{
			nospread::ApplyRecoil(lp, ucmd->viewangles, -1.0f);
		}

		if (canshoot && ucmd->buttons.test(IN_ATTACK))
		{
			if (1) // menu/nospread
				nospread::ApplySpread(ucmd->command_number, lp, ucmd->viewangles, -1.0f);

			if (1 && weapon->GetNextPrimaryFire() > curtime) // menu/autopistol
				ucmd->buttons.del(IN_ATTACK);

			if (1)
			{
				if (weapon->GetNextPrimaryFire() > curtime)
				{
					ucmd->viewangles = viewang;
				}
				else
				{
				//	sendpacket = false;
				}
			}

			if (weapon->GetNextPrimaryFire() <= curtime)
				aimbot::Next();
		}


		Angle move;
		VectorAngles(ucmd->move, move);

		float mvel = ucmd->move.Length2D();
		float rsin, rcos;
		SinCos(Deg2Rad(ucmd->viewangles.y - viewang.y + move.y), rsin, rcos);

		if (ucmd->viewangles.x < -90.0f || ucmd->viewangles.x > 90.0f)
		{
			ucmd->move.x = rsin * mvel;
			ucmd->move.y = rcos * mvel;
		}
		else
		{
			ucmd->move.x = rcos * mvel;
			ucmd->move.y = rsin * mvel;
		}

		// ucmd->move.RotateInPlace();
	}
}

void (__thiscall *org_RunCommand)(void *, Entity *, UserCmd *, void *);
void __fastcall hooked_RunCommand(void *t, void *, Entity *pl, UserCmd *ucmd, void *movehelper)
{
	if (pl == LocalPlayer())
	{
		aimbot::movehelper = movehelper;

		if (ucmd && speedfix[ucmd->command_number & (sizeof(speedfix)-1)])
		{
			return aimbot::RunCommand(ucmd);
		}
	}

	org_RunCommand(t, pl, ucmd, movehelper);
}

extern "C" int __stdcall start(void *, int r, void *)
{
	if (r == 1)
	{
		client = new IClient();
		dtmgr::Start( );

		dtmgr::SetHook("DT_LocalPlayerExclusive", "m_nTickBase", [] (const RecvProxyData &data, void *t, RecvProxyResult &out)
		{
			if (t == LocalPlayer())
			{
				static int sim;

				out.i32 = data.value.i32 == sim ? ((Entity *)t)->GetTickCount() + 1 : data.value.i32;
				sim     = data.value.i32;
			}
			else
			{
				out.i32 = data.value.i32;
			}
		});


#if defined(VORANGEBOX)
#  if defined(GMOD)
		globals = **(Globals ***)util::FindPattern(client->GetMethod<void *>(0), 0x100, Q"89 0D .? ? ? ? E8");
#  else
#		error OB Globals signature is unknown
#  endif
#else
		globals = **(Globals ***)util::FindPattern(client->GetMethod<void *>(0), 0x100, Q"A3 .? ? ? ? E8");
#endif

		engine  = new IEngine;

		ents 	= new IEntities;
		mdlinfo = new IModelInfo;

		enginetrace = new ITrace;

		movement   = new IMovement;
		prediction = new IPrediction;

		surface    = new ISurface;
		enginevgui = new IEngineVGui;
		ui::Start( );


#if defined(VORANGEBOX)
		engine->SetHook(20,     (void *)hooked_SetViewAngles, &org_SetViewAngles);
		enginevgui->SetHook(13, (void *)hooked_Paint, &org_Paint);
		prediction->SetHook(17, (void *)hooked_RunCommand, &org_RunCommand);
#endif

#if defined(VL4D)
		engine->SetHook(20,     (void *)hooked_SetViewAngles, &org_SetViewAngles);
		enginevgui->SetHook(14, (void *)hooked_Paint, &org_Paint);
		prediction->SetHook(18, (void *)hooked_RunCommand, &org_RunCommand);
#endif
	}

	return 1;
}