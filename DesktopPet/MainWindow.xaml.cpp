#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Windowing;
using namespace Windows::Foundation;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace
{
    // quotes.json 读取失败时的兜底语料
    constexpr wchar_t const* kDefaultQuotes = LR"JSON({
  "greetings": ["喵！我是小橘，今天也陪你一起～"],
  "feed": ["唔姆唔姆……小鱼干真好吃！"],
  "walk": ["出发！左三圈右三圈～"],
  "fallback": ["喵？"],
  "rules": [
    { "keywords": ["你好", "hello", "hi"], "answers": ["你好呀！今天过得怎么样？"] },
    { "keywords": ["名字", "你是谁"], "answers": ["我叫小橘，是住在你桌面上的小猫咪！"] }
  ]
})JSON";
}

namespace winrt::DesktopPet::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        m_rng.seed(std::random_device{}());
        m_start = steady_clock::now();
        m_nextBlink = m_start + 2500ms;
        m_nextDecision = m_start + 4s;

        ConfigureWindow();
        LoadQuotes();

        m_timer = DispatcherQueue().CreateTimer();
        m_timer.Interval(33ms);
        m_timer.Tick({ this, &MainWindow::OnTick });
        m_timer.Start();

        Controls::Canvas::SetLeft(Pet(), m_petX);
        Controls::Canvas::SetTop(Pet(), m_petY);
    }

    void MainWindow::ConfigureWindow()
    {
        auto appWindow = AppWindow();

        // 让窗口铺满屏幕底部的一条区域
        auto display = DisplayArea::GetFromWindowId(appWindow.Id(), DisplayAreaFallback::Primary);
        auto workArea = display.WorkArea();
        int const winH = 260;
        appWindow.MoveAndResize({ workArea.X, workArea.Y + workArea.Height - winH, workArea.Width, winH });

        if (auto presenter = appWindow.Presenter().try_as<OverlappedPresenter>())
        {
            presenter.SetBorderAndTitleBar(false, false);
            presenter.IsAlwaysOnTop(true);
            presenter.IsResizable(false);
            presenter.IsMaximizable(false);
            presenter.IsMinimizable(false);
        }
        Title(L"DesktopPet");

        // 分层窗口 + 颜色键：品红像素完全透明且点击穿透
        HWND hwnd{};
        if (auto native = this->try_as<::IWindowNative>())
        {
            native->get_WindowHandle(&hwnd);
        }
        if (hwnd)
        {
            auto const ex = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
            SetWindowLongPtrW(hwnd, GWL_EXSTYLE,
                ex | gsl::narrow<LONG_PTR>(WS_EX_LAYERED | WS_EX_TOOLWINDOW));
            SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 255, LWA_COLORKEY);
        }

        m_floorY = static_cast<double>(winH - 180 - 12);
        m_petY = m_floorY;
    }

    fire_and_forget MainWindow::LoadQuotes()
    {
        auto lifetime = get_strong();
        try
        {
            auto folder = Windows::ApplicationModel::Package::Current().InstalledLocation();
            auto file = co_await folder.GetFileAsync(L"data\\quotes.json");
            auto text = co_await Windows::Storage::FileIO::ReadTextAsync(file);
            m_quotes = nlohmann::json::parse(winrt::to_string(text));
        }
        catch (...)
        {
            m_quotes = nlohmann::json::parse(kDefaultQuotes);
        }
        ShowBubble(Pick("greetings"));
    }

    std::wstring MainWindow::Pick(char const* key)
    {
        try
        {
            auto const& arr = m_quotes.at(key);
            std::uniform_int_distribution<size_t> dist(0, arr.size() - 1);
            return std::wstring{ winrt::to_hstring(arr[dist(m_rng)].get<std::string>()).c_str() };
        }
        catch (...)
        {
            return L"喵？";
        }
    }

    void MainWindow::ShowBubble(std::wstring const& text)
    {
        BubbleText().Text(winrt::hstring{ text });
        Bubble().Visibility(Visibility::Visible);
        m_bubbleUntil = steady_clock::now() + 4s;
    }

    void MainWindow::ReplyTo(std::wstring const& raw)
    {
        auto text = winrt::to_string(raw); // UTF-8，与 quotes.json 中的关键词一致
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        try
        {
            for (auto const& rule : m_quotes.at("rules"))
            {
                for (auto const& kw : rule.at("keywords"))
                {
                    if (text.find(kw.get<std::string>()) != std::string::npos)
                    {
                        auto const& answers = rule.at("answers");
                        std::uniform_int_distribution<size_t> dist(0, answers.size() - 1);
                        ShowBubble(std::wstring{
                            winrt::to_hstring(answers[dist(m_rng)].get<std::string>()).c_str() });
                        return;
                    }
                }
            }
        }
        catch (...)
        {
        }
        ShowBubble(Pick("fallback"));
    }

    void MainWindow::OnTick(DispatcherQueueTimer const&, IInspectable const&)
    {
        auto const now = steady_clock::now();
        double const t = duration<double>(now - m_start).count();

        // 眨眼
        bool const blinking = (now >= m_blinkStart) && (now < m_blinkStart + 120ms);
        double const eyeY = blinking ? 0.12 : 1.0;
        LeftEyeScale().ScaleY(eyeY);
        RightEyeScale().ScaleY(eyeY);
        if (now >= m_nextBlink)
        {
            m_blinkStart = now;
            std::uniform_real_distribution<double> gap(2.0, 6.0);
            m_nextBlink = now + duration_cast<steady_clock::duration>(duration<double>(gap(m_rng)));
        }

        // 被喂食后的开心弹跳
        double bounce = 1.0;
        if (now < m_bounceUntil)
        {
            double const p = duration<double>(m_bounceUntil - now).count();
            bounce = 1.0 + 0.12 * std::abs(std::sin(p * 12.0));
        }
        PetScale().ScaleY(bounce);

        // 行走 / 站立浮动
        if (!m_dragging)
        {
            if (m_walking)
            {
                double const dx = m_targetX - m_petX;
                double const step = 3.0;
                if (std::abs(dx) <= step)
                {
                    m_petX = m_targetX;
                    m_walking = false;
                }
                else
                {
                    m_petX += (dx > 0 ? step : -step);
                    PetScale().ScaleX(dx > 0 ? 1.0 : -1.0);
                }
            }
            else if (now >= m_nextDecision)
            {
                std::uniform_real_distribution<double> roll(0.0, 1.0);
                if (roll(m_rng) < 0.4)
                {
                    double const stageW = (std::max)(200.0, Stage().ActualWidth());
                    std::uniform_real_distribution<double> target(10.0, stageW - 160.0);
                    m_targetX = target(m_rng);
                    m_walking = true;
                }
                std::uniform_real_distribution<double> wait(3.0, 8.0);
                m_nextDecision = now + duration_cast<steady_clock::duration>(duration<double>(wait(m_rng)));
            }

            double const bob = m_walking ? 3.0 * std::sin(t * 14.0) : 2.0 * std::sin(t * 2.5);
            Controls::Canvas::SetLeft(Pet(), m_petX);
            Controls::Canvas::SetTop(Pet(), m_petY + bob);
        }

        // 气泡跟随与自动隐藏
        if (Bubble().Visibility() == Visibility::Visible)
        {
            Controls::Canvas::SetLeft(Bubble(), m_petX - 40);
            Controls::Canvas::SetTop(Bubble(), m_petY - 46);
            if (now >= m_bubbleUntil)
            {
                Bubble().Visibility(Visibility::Collapsed);
            }
        }
    }

    void MainWindow::Stage_SizeChanged(IInspectable const&, SizeChangedEventArgs const& e)
    {
        m_floorY = (std::max)(0.0, e.NewSize().Height - 180.0 - 12.0);
        if (!m_dragging)
        {
            m_petY = m_floorY;
        }
    }

    void MainWindow::Pet_PointerPressed(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!e.GetCurrentPoint(Stage()).Properties().IsLeftButtonPressed())
        {
            return; // 右键交给 ContextFlyout
        }
        m_dragging = true;
        m_walking = false;
        Pet().CapturePointer(e.Pointer());
        auto const pt = e.GetCurrentPoint(Stage()).Position();
        m_dragOffX = pt.X - m_petX;
        m_dragOffY = pt.Y - m_petY;
        e.Handled(true);
    }

    void MainWindow::Pet_PointerMoved(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_dragging)
        {
            return;
        }
        auto const pt = e.GetCurrentPoint(Stage()).Position();
        double const w = Stage().ActualWidth();
        double const h = Stage().ActualHeight();
        m_petX = std::clamp(pt.X - m_dragOffX, 0.0, (std::max)(0.0, w - 150.0));
        m_petY = std::clamp(pt.Y - m_dragOffY, 0.0, (std::max)(0.0, h - 180.0));
        Controls::Canvas::SetLeft(Pet(), m_petX);
        Controls::Canvas::SetTop(Pet(), m_petY);
        e.Handled(true);
    }

    void MainWindow::Pet_PointerReleased(IInspectable const&, PointerRoutedEventArgs const& e)
    {
        if (!m_dragging)
        {
            return;
        }
        m_dragging = false;
        Pet().ReleasePointerCapture(e.Pointer());
        m_petY = m_floorY; // 落地
        e.Handled(true);
    }

    void MainWindow::Menu_Chat(IInspectable const&, RoutedEventArgs const&)
    {
        auto const panel = ChatPanel();
        panel.Visibility(panel.Visibility() == Visibility::Visible
            ? Visibility::Collapsed : Visibility::Visible);
    }

    void MainWindow::Menu_Feed(IInspectable const&, RoutedEventArgs const&)
    {
        m_bounceUntil = steady_clock::now() + 900ms;
        ShowBubble(Pick("feed"));
    }

    void MainWindow::Menu_Walk(IInspectable const&, RoutedEventArgs const&)
    {
        double const stageW = (std::max)(200.0, Stage().ActualWidth());
        std::uniform_real_distribution<double> target(10.0, stageW - 160.0);
        m_targetX = target(m_rng);
        m_walking = true;
        ShowBubble(Pick("walk"));
    }

    void MainWindow::Menu_Quit(IInspectable const&, RoutedEventArgs const&)
    {
        Close();
    }

    void MainWindow::Send_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto const text = InputBox().Text();
        if (text.empty())
        {
            return;
        }
        InputBox().Text(L"");
        ReplyTo(std::wstring{ text.c_str() });
    }

    void MainWindow::InputBox_KeyDown(IInspectable const&, KeyRoutedEventArgs const& e)
    {
        if (e.Key() == Windows::System::VirtualKey::Enter)
        {
            Send_Click(nullptr, {});
            e.Handled(true);
        }
    }

    fire_and_forget MainWindow::Mic_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto lifetime = get_strong();
        MicButton().IsEnabled(false);
        try
        {
            namespace sr = Windows::Media::SpeechRecognition;
            sr::SpeechRecognizer recognizer{ Windows::Globalization::Language{ L"zh-CN" } };
            co_await recognizer.CompileConstraintsAsync();
            auto result = co_await recognizer.RecognizeWithUIAsync();
            if (result && result.Status() == sr::SpeechRecognitionResultStatus::Success)
            {
                ReplyTo(std::wstring{ result.Text().c_str() });
            }
            else
            {
                ShowBubble(L"没听清，再跟我说一次？");
            }
        }
        catch (...)
        {
            ShowBubble(L"这台设备上语音识别不可用，打字跟我聊也可以～");
        }
        MicButton().IsEnabled(true);
    }
}
