#include "room_dialog.hpp"

#include <windows.h>

namespace {
    constexpr int kCreateButtonId = 101;
    constexpr int kJoinButtonId = 102;
    constexpr int kCancelButtonId = 103;
    constexpr int kRoomCodeEditId = 104;
    constexpr int kUsernameEditId = 105;
    constexpr int kPasswordEditId = 106;

    const COLORREF kCream = RGB(245, 235, 214);
    const COLORREF kBrownText = RGB(92, 64, 51);

    struct DialogState {
        LoginPrompt result{"", "", MatchChoice{MatchIntent::FindMatch, std::nullopt}};
        HWND editUsername = nullptr;
        HWND editPassword = nullptr;
        HWND editRoomCode = nullptr;
        HBRUSH backgroundBrush = nullptr;
    };

    std::string narrow(HWND edit) {
        wchar_t buffer[64] = {};
        GetWindowTextW(edit, buffer, 64);
        std::wstring wide(buffer);
        std::string result;
        result.reserve(wide.size());
        for (wchar_t c : wide) result.push_back(static_cast<char>(c));
        return result;
    }

    LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (msg == WM_CREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            auto* state = reinterpret_cast<DialogState*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

            CreateWindowExW(0, L"STATIC", L"Username:",
                WS_CHILD | WS_VISIBLE, 20, 15, 80, 20, hwnd, nullptr, cs->hInstance, nullptr);
            state->editUsername = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 13, 130, 22, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUsernameEditId)), cs->hInstance, nullptr);

            CreateWindowExW(0, L"STATIC", L"Password:",
                WS_CHILD | WS_VISIBLE, 20, 45, 80, 20, hwnd, nullptr, cs->hInstance, nullptr);
            state->editPassword = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD, 110, 43, 130, 22, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kPasswordEditId)), cs->hInstance, nullptr);

            CreateWindowExW(0, L"STATIC", L"Choose how to start a game:",
                WS_CHILD | WS_VISIBLE, 20, 80, 240, 20, hwnd, nullptr, cs->hInstance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Create Room",
                WS_CHILD | WS_VISIBLE, 20, 110, 100, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCreateButtonId)), cs->hInstance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Join Room",
                WS_CHILD | WS_VISIBLE, 130, 110, 100, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kJoinButtonId)), cs->hInstance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Room code:",
                WS_CHILD | WS_VISIBLE, 20, 155, 80, 20, hwnd, nullptr, cs->hInstance, nullptr);
            state->editRoomCode = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | WS_BORDER, 110, 153, 130, 22, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRoomCodeEditId)), cs->hInstance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Cancel (Quick Match)",
                WS_CHILD | WS_VISIBLE, 20, 190, 210, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelButtonId)), cs->hInstance, nullptr);

            SetFocus(state->editUsername);
            return 0;
        }

        auto* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLOREDIT: {
                HDC hdc = reinterpret_cast<HDC>(wParam);
                SetTextColor(hdc, kBrownText);
                SetBkColor(hdc, kCream);
                return reinterpret_cast<LRESULT>(state->backgroundBrush);
            }
            case WM_COMMAND: {
                int controlId = LOWORD(wParam);
                if (controlId == kCreateButtonId || controlId == kJoinButtonId || controlId == kCancelButtonId) {
                    std::string username = narrow(state->editUsername);
                    std::string password = narrow(state->editPassword);
                    if (username.empty() || password.empty()) {
                        MessageBoxW(hwnd, L"Please enter username and password first.", L"KungFu Chess", MB_OK | MB_ICONWARNING);
                        return 0;
                    }

                    MatchChoice choice{MatchIntent::FindMatch, std::nullopt};
                    if (controlId == kCreateButtonId) {
                        choice = MatchChoice{MatchIntent::CreateRoom, std::nullopt};
                    } else if (controlId == kJoinButtonId) {
                        std::string roomCode = narrow(state->editRoomCode);
                        if (roomCode.empty()) {
                            MessageBoxW(hwnd, L"Please type a room code first.", L"KungFu Chess", MB_OK | MB_ICONWARNING);
                            return 0;
                        }
                        choice = MatchChoice{MatchIntent::JoinRoom, roomCode};
                    }

                    state->result = LoginPrompt{username, password, choice};
                    DestroyWindow(hwnd);
                }
                return 0;
            }
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

LoginPrompt promptLogin() {
    const wchar_t* className = L"KungFuChessLoginDialog";
    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    DialogState state;
    state.backgroundBrush = CreateSolidBrush(kCream);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DialogProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;
    wc.hbrBackground = state.backgroundBrush;
    wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, className, L"KungFu Chess",
        WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 270, 270,
        nullptr, nullptr, hInstance, &state);
    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(state.backgroundBrush);
    UnregisterClassW(className, hInstance);
    return state.result;
}
