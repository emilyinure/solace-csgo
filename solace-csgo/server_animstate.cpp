#include "server_animstate.h"
#include "includes.h"
void server_anim_state::update(ang_t ang) {
	static auto addr = util::find("server.dll", "55 8B EC 83 E4 ? 83 EC ? 56 57 8B F9 F3 0F 11 54 24 ?");

	if (!addr)
		return;

	__asm {
		push  0
		mov	  ecx, this
		movss xmm1, dword ptr[ang + 4]
		movss xmm2, dword ptr[ang]
		call  addr
	}
}
void server_anim_state::reset() {
	using ResetAnimState_t = void(__thiscall*)(server_anim_state*);
	static auto addr = (ResetAnimState_t)util::find("server.dll", "55 8B EC 83 EC ? 56 8B F1 57 6A ? 8B 4E ?");
	if (!addr)
		return;

	addr(this);
}
void server_anim_state::init(player_t* ent) {
	static auto addr = util::find("server.dll", "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46 ? ? ? ? ?");
	if (!addr)
		return;
	reinterpret_cast<void* (__thiscall*) (void* _this, player_t * a2)>(addr)(this, ent);
}
server_anim_state* CreateCSGOPlayerAnimstate(player_t* pEntity)
{
	//static auto func = ((address)util::find("server.dll", "E8 ? ? ? ? 89 87 ? ? ? ? 80 BF ? ? ? ? ?")).rel32< uintptr_t >(0x1);
	//static auto func = util::find("server.dll", "56 8B F1 85 F6 74 ? 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 ? 6A ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 56 E8 ? ? ? ? 83 C4 ? 8B F0");
	//server_anim_state* ret;
	//_asm {
	//	mov     ecx, pEntity;
	//	call func
	//	mov ret, eax
	//}
	//auto ret = reinterpret_cast<server_anim_state * (__thiscall*) (void* _this)>(func)(pEntity);
	//return ret;
	auto* ret = (server_anim_state *)g.m_interfaces->mem_alloc()->alloc(0x2C4);
	ret->init(pEntity);
	return ret;
}