#include "FSM_Generator.cpp"
#include "TB_Generator.cpp"

int main()
{
    FSM FSM1;
    TB TB1;
    FSM1.rf_name = "FSM_Control.txt";
    FSM1.append_data();
    FSM1.get_io_names();
    FSM1.Write_FSM();

    TB1.fileName = FSM1.module_name[0] +".sv";
    TB1.ClockName = FSM1.clock_name[0];
    TB1.ResetName = FSM1.reset_name[0];
    TB1.PS = FSM1.PS;
    TB1.NS = FSM1.NS;
    TB1.Stimulus =FSM1.stim;
    TB1.final_states = FSM1.final_states;
    TB1.reset_state = FSM1.reset_state[0];
        
    TB1.read_file();
    TB1.write_file();

    for(int i = 0; i< TB1.closed_loops.size();i++)
    {    
        for(int j = 0; j< TB1.closed_loops[i].size();j++)
            cout<<"["+to_string(i)+"] "<<"["+to_string(j)+"] "<<TB1.closed_loops[i][j]<<" ";
        cout <<endl;
    }
    for(int i = 0; i< TB1.jump_conditions.size();i++)
    {   cout<<"Jump conditions: "<<endl;
        for(int j = 0; j< TB1.jump_conditions[i].size();j++)
            cout<<"["+to_string(i)+"] "<<"["+to_string(j)+"] "<<TB1.jump_conditions[i][j]<<" ";
        cout <<endl;
    } 

}