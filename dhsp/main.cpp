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
    //      実行処理 (命令実行時に呼ばれます)
    //
    code_next();                            // 次のコードを取得(最初に必ず必要です)

    switch (cmd) {                         // サブコマンドごとの分岐
        default:
            puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }
    return RUNMODE_RUN;
}


/*------------------------------------------------------------*/

static int ref_ival;                        // 返値のための変数

static void* reffunc(int* type_res, int cmd)
{
    //      関数・システム変数の実行処理 (値の参照時に呼ばれます)
    //
    //          '('で始まるかを調べる
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != '(') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();


    switch (cmd) {                         // サブコマンドごとの分岐

    case 0x00:                              // newcmd

        p1 = code_geti();               // 整数値を取得(デフォルトなし)
        ref_ival = p1 * 2;              // 返値をivalに設定
        break;

    default:
        puterror(HSPERR_UNSUPPORTED_FUNCTION);
    }

    //          '('で終わるかを調べる
    //
    if (*type != TYPE_MARK) puterror(HSPERR_INVALID_FUNCPARAM);
    if (*val != ')') puterror(HSPERR_INVALID_FUNCPARAM);
    code_next();

    *type_res = HSPVAR_FLAG_INT;            // 返値のタイプを整数に指定する
    return (void*)&ref_ival;
}


/*------------------------------------------------------------*/

static int termfunc(int option)
{
    //      終了処理 (アプリケーション終了時に呼ばれます)
    //
    if (bot) {
        delete bot; // botのメモリを解放
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
    //      イベント処理 (HSPイベント発生時に呼ばれます)
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
    //      DLLエントリー (何もする必要はありません)
    //
    return TRUE;
}


EXPORT void WINAPI hsp3cmdinit(HSP3TYPEINFO* info)
{
    //      プラグイン初期化 (実行・終了処理を登録します)
    //
    hsp3sdk_init(info);           // SDKの初期化(最初に行なって下さい)
    info->cmdfunc = cmdfunc;        // 実行関数(cmdfunc)の登録
    info->reffunc = reffunc;        // 参照関数(reffunc)の登録
    info->termfunc = termfunc;      // 終了関数(termfunc)の登録

    /*
    //  イベントコールバックを発生させるイベント種別を設定する
    info->option = HSPEVENT_ENABLE_GETKEY;
    info->eventfunc = eventfunc;    // イベント関数(eventfunc)の登録
    */
}
/*------------------------------------------------------------*/
/*
        以下メイン
*/
/*------------------------------------------------------------*/

// プラグイン登録関数
EXPORT void WINAPI hsp3hpi_init(HSP3TYPEINFO* info)
{
    hsp3sdk_init(info);		// SDKの初期化
    info->cmdfunc = cmdfunc;	// 実行関数(cmdfunc)の登録
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
