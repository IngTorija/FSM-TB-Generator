#include <iostream> // std::cout
#include <stdio.h>
#include <fstream>  // std::ifstream
#include <sstream>  // std::stringstream
#include <regex>
#include <list>
#include <iterator>
#include <string>
#include <vector>
#include <math.h>      
using namespace std;

string module_name_str = "name.=.(.+)";//"name.=.(.+?(?=\\r))";
string clock_name_str = "clock.=.(.+)";//?(?=\\r))";
string reset_name_str = "reset.=.(.+)";//?(?=\\r))";
string states_str = "states.=.\\((.+)\\)";
string inputs_str = "inputs.=.\\((.+)\\)";
string outputs_str = "outputs.=.\\((.+)\\)";
string transitions_str = ".(\\w+).(\\w+)\\((.+?(?=\\)))";
string s_outputs_str = ".(\\w+)>\\((.+)\\)\\((.+)\\)";//".(\\w+)>\\((.+)\\)";
string outputs_name_str =  "(\\w+)\\[(\\d+)\\]";
string inputs_name_str =  "(\\w+)\\[(\\d+)\\]";
string final_states_str = "final states.=.\\((.+)\\)";
string reset_state_str = "reset state.=.(.+)";

regex module_name_re(module_name_str);
regex clock_name_re(clock_name_str);
regex reset_name_re(reset_name_str);
regex states_re (states_str);
regex inputs_re (inputs_str);
regex outputs_re (outputs_str); 
regex transitions_re (transitions_str); 
regex s_outputs_re (s_outputs_str);
regex outputs_name_re (outputs_name_str);
regex inputs_name_re (inputs_name_str);
regex final_states_re(final_states_str);
regex reset_state_re(reset_state_str);
class FSM
{
public:
    vector<string> module_name;
    vector<string> clock_name;
    vector<string> reset_name;
    vector<string> reset_state;
    
    vector<string> states;
    vector<string> inputs;
    vector<string> inputs_length;
    vector<string> outputs;
    vector<string> outputs_length;
    vector<string> transitions;
    vector<string> PS; //  [S0,S0]
    vector<string> NS; // [S0,S1]
    vector<string> stim;// [(P=0,L=1),P=1]  
    vector<string> stim_o;      
    vector<string> s_outputs; // [Q=1,Q=0]
    vector<string> s_outputs_conditions;
    vector<string> final_states;
    /* File to read: */
    string rf_name;
    /* File to write: */
    ofstream fsm_write;

    FSM();
    ~FSM();

    vector<string> get_string(string, regex,int);
    void append_data(void);
    vector<string> separate(vector<string>);
    void Write_FSM(void);
    void get_io_names(void);
    vector<string> Replace_conditions(vector<string>,char,string);
};
//Constructor:
FSM::FSM(){
}
//Destructor:
FSM::~FSM(){
}
vector<string> FSM::get_string(string fileName, regex pattern, int pattern_case){   
    vector<string> temp;
    string line_temp;
    ifstream f;
    f.open(fileName);
    if (f.good())
    {
        while (!f.eof())
        {
            getline(f, line_temp);
            smatch matches;
            if (regex_search(line_temp, matches, pattern))
            {
                //cout <<"Match" <<matches.str()<<endl;
                
                switch (pattern_case){
                case 0:
                    temp.push_back(matches[1].str());
                    break;
                case 1:
                    temp.push_back(matches[2].str());    //Group assignation of transitions                
                    break;
                case 2:
                    temp.push_back(matches[3].str());
                    break;
                }
            }
        }
        return temp;
    }
    return temp;
}

void FSM::get_io_names(){
    for (int i = 0 ; i < inputs.size() ; i++){
        smatch matches;  
        if (regex_search(inputs[i], matches, inputs_name_re))
        {   
            inputs[i] = matches[1];
            inputs_length.push_back(matches[2].str());
            //cout << "\nMATCHES[2]: " <<matches[2].str();         
        }
        else{
            inputs_length.push_back("1");
        }
    }  
    for (int i = 0 ; i < outputs.size() ; i++){
        smatch matches;  
        if (regex_search(outputs[i], matches, inputs_name_re))
        {   
            outputs[i] = matches[1];
            outputs_length.push_back(matches[2].str());      
        }
        else{
            outputs_length.push_back("1");
        }
    }  
}

vector<string> FSM::separate(vector<string> names){
    string temp = names.at(0);
    vector<string> v_temp;
    names.clear();
    stringstream ss(temp);
    while( ss.good() )
    {
        string substr;
        getline( ss, substr, ',' );
        v_temp.push_back( substr );
    }
    return v_temp;
}

void FSM::append_data(){
    module_name = get_string(rf_name, module_name_re,0);
    clock_name = get_string(rf_name, clock_name_re,0);
    reset_name = get_string(rf_name, reset_name_re,0);
    
    states = get_string(rf_name, states_re,0);
    states = separate(states);
    inputs = get_string(rf_name, inputs_re,0);
    inputs = separate(inputs);

    outputs = get_string(rf_name, outputs_re,0);
    outputs = separate(outputs);

    PS = get_string(rf_name, transitions_re,0);
    NS = get_string(rf_name, transitions_re,1);
    stim = get_string(rf_name, transitions_re,2);
   
    s_outputs = (get_string(rf_name, s_outputs_re,1));
    s_outputs_conditions = get_string(rf_name,s_outputs_re,2);

    final_states = get_string(rf_name, final_states_re,0);
    final_states = separate(final_states);

    reset_state = get_string(rf_name,reset_state_re,0);
    
}

vector<string> FSM::Replace_conditions(vector<string> vector_to_replace,char to_replace,string replace){
    vector<string> vector_replaced;
    for (int i = 0 ; i < vector_to_replace.size() ; i++){
        string temp;
        for (int j = 0 ; j < vector_to_replace[i].size() ; j++){
            if(vector_to_replace[i][j] == to_replace)
                temp = temp + replace;
            else temp = temp + vector_to_replace[i][j];
        }
        vector_replaced.push_back(temp);        
    }
    return vector_replaced;
}

void FSM::Write_FSM(){
    string fileName = module_name[0] + ".sv";
    fsm_write.open(fileName);

    //Write module name  
    fsm_write << "module "+ module_name[0] + "( \n";
    //Write Clock and reset
    fsm_write << "  input "+ clock_name[0] +", \n" + "  input " + reset_name[0] + ",\n";
    //Write Inputs
    for(int i = 0 ; i<inputs.size() ; i++){
        if(inputs_length[i] != "1")
            fsm_write << "  input ["+ to_string(stoi(inputs_length[i])-1) + ":0] " + inputs[i] << ",\n";
        else 
            fsm_write << "  input " + inputs[i] << ",\n";
    }
    //Write Outputs
    for(int i = 0 ; i<outputs.size() ; i++){
        if(i<outputs.size()-1)
            if(outputs_length[i] != "1")
                fsm_write << "  output reg ["+ to_string(stoi(outputs_length[i])-1) + ":0] " + outputs[i] << ",\n";
            else 
                fsm_write << "  output reg " + outputs[i] << ",\n";
        else
            if(outputs_length[i] != "1")
                fsm_write << "  output reg ["+ to_string(stoi(outputs_length[i])-1) + ":0] " + outputs[i] << "\n);\n";
            else 
                fsm_write << "  output reg " + outputs[i] << "\n);\n";
    }
    //Declare STATE and STATES parameters
    if(states.size() > 2 )
        fsm_write <<"   reg [" + to_string((int)floor(log2(states.size()-1))) + ":0] STATE;\n";
    else fsm_write << " reg STATE;\n";
    
    for (int i = 0; i < states.size() ; i++)
        fsm_write << "   parameter "<< states[i] + " = " <<to_string((int)floor(log2(states.size()-1))+1)+"'d" + to_string(i)+";"<<endl;
    //Write First Process: Next State
    fsm_write << "\n    always @(posedge " + clock_name[0] + " or posedge " + reset_name[0] + ") begin\n";
    fsm_write << "      if(rst)\n           STATE <= " + states[0] + ";\n";
    fsm_write << "      else\n";
    fsm_write << "          case(STATE)";
    stim_o = stim;
    stim = Replace_conditions(stim,'='," == ");
    stim = Replace_conditions(stim,','," && ");

    int l = 1;
    vector<int> trans_index;// [2,2,2,2]
    for(int j = 0; j <PS.size();j++) //J=2 PS size = 3 
    {   
        if(j != PS.size()-1){ //J!=3-2 =1
            if(PS[j] == PS[j+1])
                l++;
            else {
                trans_index.push_back(l);
                l = 1;
            }
        }else
            trans_index.push_back(l);
    
    }
    //for(int j = 0; j <trans_index.size();j++)
      //  cout << trans_index[j]<< endl;
    //trans_index = [2,2,2,2]
    int j = 0; //index for PS
    int i = 0; //index for PS length
    int k = 0; //Iterations number
    l = 0; //index NS and Stim
    while(j < PS.size())
    {
        fsm_write << "\n                  " + PS[j]+":";
        
        for(int k=0; k<trans_index[i] ; k++)
        {
            if(k== 0)
            {
                if(stim[l]!="-")
                {
                    fsm_write << "\n                    if(" + stim[l] + ")";
                    fsm_write << "\n                        STATE <= " + NS[l] +";" ;    
                }
                else
                {
                    fsm_write << "\n                        STATE <= " + NS[l] +";" ;

                }
            }
            else
            {
                if(stim[l] != "-")
                {
                    fsm_write << "\n                    else if(" + stim[l] + ")";
                    fsm_write << "\n                        STATE <= " + NS[l] + ";";
                }
                else
                {
                    fsm_write << "\n                    else";
                    fsm_write << "\n                        STATE <= " + NS[l] +";" ; 
                }
            }
            l +=1; 
        }
        j += trans_index[i]; // s0 ->s1 ->s2 ->...
        i++;   
    }
    
    fsm_write << "\n          endcase";
    fsm_write << "\n    end";

    //Write Second Process: Outputs State
    fsm_write << "\n\n    always @ (STATE) begin"<<endl;
    fsm_write << "      case(STATE)"<<endl;
    
    //s_outputs = Replace_conditions(s_outputs);
    s_outputs = Replace_conditions(s_outputs,',',";\n             ");
    for (int i = 0 ; i < states.size() ; i++)
    {   if(s_outputs_conditions[i]!="-")
        {
            fsm_write << "          " + states[i]+":"<<endl;
            fsm_write << "              if("+s_outputs_conditions[i]+") begin\n";
            fsm_write << "                  "+s_outputs[i] +";\n              end" <<endl;
        }
        else
        {
            fsm_write << "          " + states[i]+":\n              begin"<<endl;
            fsm_write << "              "+s_outputs[i] +";\n              end" <<endl;

        }
    }
    fsm_write << "      default:\n          begin\n             " +s_outputs[0]+";\n          end"<<endl;   
    fsm_write << "      endcase"<<endl;
    fsm_write << "  end"<<endl;
    fsm_write << "endmodule"<<endl;   
}

/*
int main()
{
    
    FSM FSM1;
    FSM1.rf_name = "FSM_Control.txt";
    
    FSM1.append_data();
    
    FSM1.get_io_names();
    FSM1.Write_FSM();

}*/