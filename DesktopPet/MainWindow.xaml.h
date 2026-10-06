#pragma once

#include "MainWindow.g.h"
#include <nlohmann/json.hpp>
#include <random>

namespace winrt::DesktopPet::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        void Stage_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender,
                               winrt::Microsoft::UI::Xaml::SizeChangedEventArgs const& e);
        void Pet_PointerPressed(winrt::Windows::Foundation::IInspectable const& sender,
                                winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Pet_PointerMoved(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Pet_PointerReleased(winrt::Windows::Foundation::IInspectable const& sender,
                                 winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs const& e);
        void Menu_Chat(winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Menu_Feed(winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Menu_Walk(winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Menu_Quit(winrt::Windows::Foundation::IInspectable const& sender,
                       winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void Send_Click(winrt::Windows::Foundation::IInspectable const& sender,
                        winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void InputBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        winrt::fire_and_forget Mic_Click(winrt::Windows::Foundation::IInspectable const& sender,
                                         winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        void ConfigureWindow();
        winrt::fire_and_forget LoadQuotes();
        void OnTick(winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer const& sender,
                    winrt::Windows::Foundation::IInspectable const& args);
        void ShowBubble(std::wstring const& text);
        void ReplyTo(std::wstring const& text);
        std::wstring Pick(char const* key);

        winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer m_timer{ nullptr };
        nlohmann::json m_quotes;
        std::mt19937 m_rng{};

        double m_petX{ 240.0 };
        double m_petY{ 68.0 };
        double m_floorY{ 68.0 };
        double m_targetX{ 240.0 };
        double m_dragOffX{ 0.0 };
        double m_dragOffY{ 0.0 };
        bool m_walking{ false };
        bool m_dragging{ false };

        std::chrono::steady_clock::time_point m_start{ std::chrono::steady_clock::now() };
        std::chrono::steady_clock::time_point m_blinkStart{ m_start };
        std::chrono::steady_clock::time_point m_nextBlink{ m_start };
        std::chrono::steady_clock::time_point m_bubbleUntil{ m_start };
        std::chrono::steady_clock::time_point m_bounceUntil{ m_start };
        std::chrono::steady_clock::time_point m_nextDecision{ m_start };
    };
}

namespace winrt::DesktopPet::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
