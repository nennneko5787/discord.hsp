#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <memory>
#include <unordered_set>
#include <dpp/dpp.h>
#include "hsp3plugin.h"

static dpp::cluster* bot = nullptr;
unsigned short* onReadyLabel = nullptr;

static std::unordered_set<void*> pointer_list;

/*------------------------------------------------------------*/

static int cmdfunc(int cmd)
{
    //      ���s���� (���ߎ��s���ɌĂ΂�܂�)
    //
    code_next();                            // ���̃R�[�h���擾(�ŏ��ɕK���K�v�ł�)

    switch (cmd) {                         // �T�u�R�}���h���Ƃ̕���
        default:
            puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }
    return RUNMODE_RUN;
}


/*------------------------------------------------------------*/

static int ref_ival;                        // �Ԓl�̂��߂̕ϐ�

static void* reffunc(int* type_res, int cmd)
{
    //      �֐��E�V�X�e���ϐ��̎��s���� (�l�̎Q�Ǝ��ɌĂ΂�܂�)
    //
    //          '('�Ŏn�܂邩�𒲂ׂ�
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != '(') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();


    switch (cmd) {                         // �T�u�R�}���h���Ƃ̕���

    case 0x00:                              // newcmd

        p1 = code_geti();               // �����l���擾(�f�t�H���g�Ȃ�)
        ref_ival = p1 * 2;              // �Ԓl��ival�ɐݒ�
        break;

    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }

    //          '('�ŏI��邩�𒲂ׂ�
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != ')') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();

    *type_res = HSPVAR_FLAG_INT;            // �Ԓl�̃^�C�v�𐮐��Ɏw�肷��
    return (void*)&ref_ival;
}


/*------------------------------------------------------------*/

static int termfunc(int option)
{
    //      �I������ (�A�v���P�[�V�����I�����ɌĂ΂�܂�)
    //
    if (bot) {
        delete bot; // bot�̃����������
        bot = nullptr;
    }
    for (void* ptr : pointer_list) {
        delete reinterpret_cast<dpp::user_identified*>(ptr);
    }
    pointer_list.clear();
    return 0;
}

/*------------------------------------------------------------*/

static int eventfunc(int event, int prm1, int prm2, void* prm3)
{
    //      �C�x���g���� (HSP�C�x���g�������ɌĂ΂�܂�)
    //
    switch (event) {
    case HSPEVENT_GETKEY:
    {
        int* ival;
        ival = (int*)prm3;
        *ival = 123;
        return 1;
    }
    }
    return 0;
}

/*------------------------------------------------------------*/
/*
        interface
*/
/*------------------------------------------------------------*/

int WINAPI DllMain(HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
    //      DLL�G���g���[ (��������K�v�͂���܂���)
    //
    return TRUE;
}


EXPORT void WINAPI hsp3cmdinit(HSP3TYPEINFO* info)
{
    //      �v���O�C�������� (���s�E�I��������o�^���܂�)
    //
    hsp3sdk_init(info);           // SDK�̏�����(�ŏ��ɍs�Ȃ��ĉ�����)
    info->cmdfunc = cmdfunc;        // ���s�֐�(cmdfunc)�̓o�^
    info->reffunc = reffunc;        // �Q�Ɗ֐�(reffunc)�̓o�^
    info->termfunc = termfunc;      // �I���֐�(termfunc)�̓o�^

    /*
    //  �C�x���g�R�[���o�b�N�𔭐�������C�x���g��ʂ�ݒ肷��
    info->option = HSPEVENT_ENABLE_GETKEY;
    info->eventfunc = eventfunc;    // �C�x���g�֐�(eventfunc)�̓o�^
    */
}
/*------------------------------------------------------------*/
/*
        �ȉ����C��
*/
/*------------------------------------------------------------*/

// �v���O�C���o�^�֐�
EXPORT void WINAPI hsp3hpi_init(HSP3TYPEINFO* info)
{
    hsp3sdk_init(info);		// SDK�̏�����
    info->cmdfunc = cmdfunc;	// ���s�֐�(cmdfunc)�̓o�^
    return;
}

bool WINAPI DHSP_Initialize(HSPEXINFO* pexinfo)
{
    char* _token = pexinfo->HspFunc_prm_getns();
    std::string token = std::string(_token);
    bot = new dpp::cluster(token);

    bot->on_ready([](const dpp::ready_t& event) {
        if (onReadyLabel != nullptr) {
            code_call(onReadyLabel);
        }
    });
    pexinfo->hspctx->stat = 0;
    return true;
}

bool WINAPI DHSP_OnReady(HSPEXINFO* pexinfo)
{
    onReadyLabel = pexinfo->HspFunc_prm_getlb();
    pexinfo->hspctx->stat = 0;
    return true;
}

bool WINAPI DHSP_Run(HSPEXINFO* pexinfo)
{
    if (!bot) {
        pexinfo->hspctx->stat = 1;
        return true;
    }

    bot->start(false);
    pexinfo->hspctx->stat = 0;
    return true;
}
