#include <Mahi/Gui.hpp>
#include <Mahi/Daq.hpp>
#include <Mahi/Util.hpp>
#include <Mahi/Robo/Mechatronics/AtiSensor.hpp>
#include <iostream>
#include <fstream>
#include <Windows.h>

using namespace mahi::gui;
using namespace mahi::daq;
using namespace mahi::robo;
using namespace mahi::util;

std::string base1;
std::string base2;
std::string testNum = "1";
int counter = 0;
bool isSaving = false;

struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImPlotPoint> Data;
    ScrollingBuffer() {
        MaxSize = 2000;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(double x, double y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImPlotPoint(x,y));
        else {
            Data[Offset] = ImPlotPoint(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

class AtiGui : public Application {
public:
    // constructor
    AtiGui(const std::string& serial, const ChanNums& channels) : 
        Application(640,480,"ATI GUI"),
        m_serial(serial),
        m_channels(channels)
    {
        // ImGui
        ImGui::DisableViewports();
        ImGui::DisableDocking();
        ImPlot::StyleColorsDark();
        // configure ATI
        m_ati.load_calibration(serial + ".cal");
        m_ati.set_channels(&m_daq.AI[channels[0]],
                           &m_daq.AI[channels[1]],
                           &m_daq.AI[channels[2]],
                           &m_daq.AI[channels[3]],
                           &m_daq.AI[channels[4]],
                           &m_daq.AI[channels[5]]); 
    };

    void update() {
        // buffers
        static ScrollingBuffer voltages[6];
        static ScrollingBuffer forces[3];
        static ScrollingBuffer torques[3];
        // read data
        m_daq.AI.read();
        double t = time().as_seconds();
        for (int i = 0; i < m_channels.size(); ++i)
            voltages[i].AddPoint(t, m_daq.AI[m_channels[i]]);
        forces[0].AddPoint(t, m_ati.get_force(AxisX));
        forces[1].AddPoint(t, m_ati.get_force(AxisY));
        forces[2].AddPoint(t, m_ati.get_force(AxisZ));
        torques[0].AddPoint(t, m_ati.get_torque(AxisX));
        torques[1].AddPoint(t, m_ati.get_torque(AxisY));
        torques[2].AddPoint(t, m_ati.get_torque(AxisZ));
        // gui code 
        static std::string txt_buff;
        ImGui::BeginFixed("##Window", ImVec2(0,0), ImVec2(640,480), ImGuiWindowFlags_NoTitleBar);
        ImGui::Text("ATI Sensor:      %s", m_serial.c_str());
        ImGui::Text("Sampling Rate:   %.0f Hz", ImGui::GetIO().Framerate);
        if (ImGui::Button("Zero Sensor", ImVec2(-1,0))) 
            m_ati.zero();        
        ImGui::Separator();
        if (ImGui::BeginTabBar("Tabs")) {
            if (ImGui::BeginTabItem("Force")) {
                ImPlot::SetNextPlotLimitsX(t-10,t,ImGuiCond_Always);
                ImPlot::SetNextPlotLimitsY(-10,10);
                if (ImPlot::BeginPlot("##Force","Time [s]","Force [N]",ImVec2(-1,-1))) {
                        ImPlot::PlotLine("X", &forces[0].Data[0].x, &forces[0].Data[0].y, forces[0].Data.Size, forces[0].Offset, sizeof(ImPlotPoint));
                        ImPlot::PlotLine("Y", &forces[1].Data[0].x, &forces[1].Data[0].y, forces[1].Data.Size, forces[1].Offset, sizeof(ImPlotPoint));
                        ImPlot::PlotLine("Z", &forces[2].Data[0].x, &forces[2].Data[0].y, forces[2].Data.Size, forces[2].Offset, sizeof(ImPlotPoint));
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Torque")) {
                ImPlot::SetNextPlotLimitsX(t-10,t,ImGuiCond_Always);
                ImPlot::SetNextPlotLimitsY(-100,100);
                if (ImPlot::BeginPlot("##Torque","Time [s]","Torque [N-mm]",ImVec2(-1,-1))) {
                        ImPlot::PlotLine("X", &torques[0].Data[0].x, &torques[0].Data[0].y, torques[0].Data.Size, torques[0].Offset, sizeof(ImPlotPoint));
                        ImPlot::PlotLine("Y", &torques[1].Data[0].x, &torques[1].Data[0].y, torques[1].Data.Size, torques[1].Offset, sizeof(ImPlotPoint));
                        ImPlot::PlotLine("Z", &torques[2].Data[0].x, &torques[2].Data[0].y, torques[2].Data.Size, torques[2].Offset, sizeof(ImPlotPoint));
                    ImPlot::EndPlot();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Voltage")) {
                ImPlot::SetNextPlotLimitsX(t-10,t,ImGuiCond_Always);
                ImPlot::SetNextPlotLimitsY(-10,10);
                if (ImPlot::BeginPlot("##Voltage","Time [s]","Voltage [V]",ImVec2(-1,-1))) {
                    for (int i = 0; i < 6; ++i) {
                        txt_buff = "Ch " + std::to_string(m_channels[i]);
                        ImPlot::PlotLine(txt_buff.c_str(), &voltages[i].Data[0].x, &voltages[i].Data[0].y, voltages[i].Data.Size, voltages[i].Offset, sizeof(ImPlotPoint));
                    }
                    ImPlot::EndPlot();                                      
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();

        SHORT keyState = GetKeyState(VK_SPACE);
        bool isDown = keyState & 0x8000;

        if(isDown && counter == 0) {
            isSaving = !isSaving;
            if (isSaving) {
                prepareDataFiles();  // open and prepare the file
            } else {
                myfile.close();  // close the file when stopping saving
            }
            counter = 1;
        }

        
        if (!isDown) {
            counter = 0;
        }

        if (isSaving) {
            saveData();
        }
    }

    std::ofstream myfile;  // declare globally to keep file open

    void prepareDataFiles() {
        base1 = std::string("C:/Users/mssch/Documents/Rice/Semesters/Summer 2024/MAHI Lab Research/Force Sensor Data/test_") + testNum;
        int k = 0;
        std::string temp = base1;
        while (fileExists(temp + ".csv")) {
            k++;
            temp = base1 + "_duplicate" + cStr(k);
        }
        base1 = temp + ".csv";
        myfile.open(base1, std::ofstream::app);  // open the file once here
        myfile << "Time" << "," << "Force-X" << "," << "Force-Y" << "," << "Force-Z" << "," << "Torque-X" << "," << "Torque-Y" << "," << "Torque-Z" << std::endl;
    }

    void saveData() {
        if (!myfile.is_open()) return;  // ensure file is open before writing
        double t = time().as_seconds();
        myfile << t << "," << m_ati.get_force(AxisX) << "," << m_ati.get_force(AxisY) << "," << m_ati.get_force(AxisZ) << "," << m_ati.get_torque(AxisX) << "," << m_ati.get_torque(AxisY) << "," << m_ati.get_torque(AxisZ) << std::endl;
    }


    Q8Usb       m_daq;
    ChanNums    m_channels;
    AtiSensor   m_ati;
    std::string m_serial;
};

// main
int main(int argc, char *argv[])
{
    Options options("ATI GUI", "Utility tool for inspecting ATI F/T sensor data with a Q8-USB");
    options.add_options()
        ("s,serial","ATI Sensor Serial Number (e.g. FT06833)", cxxopts::value<std::string>())
        ("c,channels","Analog input channel numbers (e.g. 0,1,2,3,4,5)", cxxopts::value<ChanNums>())
        ("h,help","Help");

    auto result = options.parse(argc, argv);

    if (result.count("h")) {
        std::cout << options.help();
        return 0;
    }

    if (result.count("s") == 0) {
        LOG(Error) << "You must provide a valid serial number for the ATI sensor (e.g. ati.exe -s FT06833)";
        std::cout << options.help();
        return -1;
    }

    std::string serial = result["s"].as<std::string>();
    ChanNums channels  = {0,1,2,3,4,5};

    if (result.count("c")) {
        channels = result["c"].as<ChanNums>();
        if (channels.size() != 6) {
            LOG(Error) << "The number of provided analog input channels must be 6";
            std::cout << options.help();
            return -1;
        }
    }

    AtiGui app(serial,channels);
    app.run();
    return 0;
}

bool fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
}

std::string cStr(int num) {
    return std::to_string(num);
}