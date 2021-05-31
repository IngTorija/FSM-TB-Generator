import re
import math

class FSM:
    def __init__(self): #ACA
        # Atributos
        self.fsm_file = ""
        self.dictionary  = {
            "module_name":"",
            "clock_name": "",
            "reset_name":"",
            "inputs":[],
            "states": [] ,
            "outputs":[],
            "transitions":[],
            "PS":[],
            "NS":[],
            "stim":[],
            "s_outputs":[],
            "s_outputs_states":[],
            "s_outputs_conditions":[],
            "s_transitions_conditions": [],
            "outputs_length": [],
            "inputs_length": [],
            }
        self.reg_exp = {
            "module_name_str": "name.=.(.+?(?=\r))",
            "clock_name_str": "clock.=.(.+?(?=\r))",
            "reset_name_str": "reset.=.(.+?(?=\r))",
            "states_str": "states.=.\((.+)\)",
            "inputs_str": "inputs.=.\((.+)\)",
            "inputs_name_str": "(\w+)\[(\d+)\]",
            "outputs_str": "outputs.=.\((.+)\)",
            "outputs_name_str": "(\w+)\[(\d+)\]",
            "transitions_str":".(\w+).(\w+)\((.+?(?=\)))",
            "s_outputs_str": ".(\w+)>\((.+)\)\((.+)\)"   
        } 
    def get_string(self,fileName,pattern_case,reg_exp):
        archivo = open(fileName,'r')
        temp = []
        for line in archivo:
            m = re.search(reg_exp,line)
            if(m!=None):
                if(pattern_case==0):
                    return str(m.group(1)).split(",")
                elif(pattern_case==1):
                    temp.append(m.group(1))
                elif(pattern_case ==2):
                    temp.append(m.group(2))
                elif(pattern_case ==3):
                    temp.append(m.group(3))
        archivo.close()
        return temp #Group assignation of state_outputs
    
    def get_io_names(self,i_list,reg_ex):
        names_temp = []
        size_temp = []
        
        for item in i_list:
            m = re.search(reg_ex,item)
            if(m!=None):
                names_temp.append(m.group(1))
                size_temp.append(m.group(2))
            else:
                names_temp.append(item)
                size_temp.append("1")
        return names_temp,size_temp

    def append_data(self):
        self.dictionary["module_name"] = self.get_string(self.fsm_file,0,self.reg_exp["module_name_str"])
        self.dictionary["clock_name"] = self.get_string(self.fsm_file,0,self.reg_exp["clock_name_str"])
        self.dictionary["reset_name"] = self.get_string(self.fsm_file,0,self.reg_exp["reset_name_str"])
        self.dictionary["inputs"] = self.get_string(self.fsm_file,0,self.reg_exp["inputs_str"])
        self.dictionary["inputs"],self.dictionary["inputs_length"] = self.get_io_names(self.dictionary["inputs"],self.reg_exp["inputs_name_str"])
        
        self.dictionary["outputs"] = self.get_string(self.fsm_file,0,self.reg_exp["outputs_str"])
        self.dictionary["outputs"],self.dictionary["outputs_length"] = self.get_io_names(self.dictionary["outputs"],self.reg_exp["outputs_name_str"])
        self.dictionary["states"] = self.get_string(self.fsm_file,0,self.reg_exp["states_str"])
        self.dictionary["PS"] = self.get_string(self.fsm_file,1,self.reg_exp["transitions_str"])
        self.dictionary["NS"] = self.get_string(self.fsm_file,2,self.reg_exp["transitions_str"])
        self.dictionary["stim"] = self.get_string(self.fsm_file,3,self.reg_exp["transitions_str"])
        
        self.dictionary["s_outputs"] = self.get_string(self.fsm_file,2,self.reg_exp["s_outputs_str"])
        self.dictionary["s_outputs_states"] = self.get_string(self.fsm_file,1,self.reg_exp["s_outputs_str"])
        self.dictionary["s_outputs_conditions"] = self.get_string(self.fsm_file,3,self.reg_exp["s_outputs_str"])
        self.dictionary["s_transitions_conditions"] = self.get_string(self.fsm_file,3,self.reg_exp["transitions_str"])
        # Replace , by && and = by ==
        for i in range(0,len(self.dictionary["s_outputs_conditions"])):
            self.dictionary["s_outputs_conditions"][i] = self.dictionary["s_outputs_conditions"][i].replace(","," && ")
            self.dictionary["s_outputs_conditions"][i] = self.dictionary["s_outputs_conditions"][i].replace("="," == ")

        for i in range(0,len(self.dictionary["s_outputs"])):
            self.dictionary["s_outputs"][i]= self.dictionary["s_outputs"][i].replace(',',';\n                 ')
    
    def get_input_data(self):# input_names = [[1,2,3,4,5,6][a,b,c,d,e]] input_names[0][2] :(
        for i in range(0, len(self.dictionary["stim"])):
            self.dictionary["stim"][i]=self.dictionary["stim"][i].replace(","," && ")
            self.dictionary["stim"][i]=self.dictionary["stim"][i].replace("="," == ")
        # print(self.dictionary["stim"])

    def write_FSM(self):
        fsm_file = open(str(self.dictionary["module_name"][0])+".sv",'w')
        #Name
        fsm_file.write("module "+self.dictionary["module_name"][0]+"(\n")
        #Sensitive list
        fsm_file.write("    input "+self.dictionary["clock_name"][0]+",\n    input "+self.dictionary["reset_name"][0]+", \n")
        for inp in range(0,len(self.dictionary["inputs"])):
            fsm_file.write("    input ")
            if(int(self.dictionary["inputs_length"][inp]) > 1):
                fsm_file.write("[" + str(int(self.dictionary["inputs_length"][inp])-1) +":0] ")
            fsm_file.write(self.dictionary["inputs"][inp]+",\n")
            
        for i in range(0,len(self.dictionary["outputs"])):
            if(i < len(self.dictionary["outputs"])-1):
                fsm_file.write("    output reg ")
                if(int(fsm1.dictionary["outputs_length"][i]) > 1):
                    fsm_file.write("[" + str(int(self.dictionary["outputs_length"][i])-1) + ":0] ")
                fsm_file.write(self.dictionary["outputs"][i]+",\n")
                
            else:
                fsm_file.write("    output reg ")
                if(int(fsm1.dictionary["outputs_length"][i]) > 1):
                    fsm_file.write("[" + str(int(self.dictionary["outputs_length"][i])-1) + ":0] ")
                fsm_file.write(self.dictionary["outputs"][i]+"    \n);\n")
 
        #State and Next State
        if(int(math.ceil(math.log(len(self.dictionary["states"]),2))+1) > 2):
            fsm_file.write("    reg [" + str(int(math.ceil(math.log(len(self.dictionary["states"]),2)-1))) + ":0] STATE;\n")
        else:
            fsm_file.write("    reg STATE;\n")
        for i in range(0,len(self.dictionary["states"])):
            fsm_file.write("    parameter "+self.dictionary["states"][i]+"="+str(int(math.ceil(math.log(len(self.dictionary["states"]),2))))+"'d"+str(i)+";\n")
        # Initializa state and  assign next state
        fsm_file.write("\n    always @"+"(posedge "+self.dictionary["clock_name"][0]+" or posedge "+self.dictionary["reset_name"][0]+")\n")
        fsm_file.write("    begin\n")
        fsm_file.write("        if("+self.dictionary["reset_name"][0]+")\n")
        fsm_file.write("            STATE <="+self.dictionary["states"][0]+";")
        fsm_file.write("\n        else\n")
        fsm_file.write("            case(STATE)\n")
        #---------------CODE FOR TRANSITIONS ALGORITHM---------------------------#
        
        self.get_input_data()
        index_states = []
        l = 1
        for j in range(0,len(self.dictionary["PS"])):
            if(j!= len(self.dictionary["PS"])-1):
                if(self.dictionary["PS"][j] == self.dictionary["PS"][j+1]):
                    l = l + 1
                else:
                    index_states.append(l)
                    l = 1
            else:
                index_states.append(l)
        print(self.dictionary["PS"])
        print(self.dictionary["s_transitions_conditions"])
        print(self.dictionary["NS"])
        print(index_states)
        i = 0   #index for index_states
        j = 0   #index for PS (0,4)
        k = 0   #Index iterations number
        l = 0   #Index for NS and Stim (0,8)

        while (j < len(self.dictionary["PS"])):
            fsm_file.write("\n                "+self.dictionary["PS"][j]+":")
            for k in range(0,index_states[i]):
                if(k==0):
                    if(self.dictionary["s_transitions_conditions"][l]!="-"):    
                        fsm_file.write("\n                    if("+self.dictionary["stim"][l]+")")
                        fsm_file.write("\n                        STATE <= " + self.dictionary["NS"][l] + ";")
                    else:
                        fsm_file.write("\n                        STATE <= " + self.dictionary["NS"][l] + ";")
                else:
                    if(self.dictionary["s_transitions_conditions"][l]!="-"):    
                        fsm_file.write("\n                    else if("+self.dictionary["stim"][l]+")")
                        fsm_file.write("\n                        STATE <= " + self.dictionary["NS"][l]+";")
                    else:
                        fsm_file.write("\n                    else ")
                        fsm_file.write("\n                        STATE <= " + self.dictionary["NS"][l]+";")
                l += 1
            j += index_states[i]
            i += 1
        
        
     
        #-----------END----------------------------------------------------------
        fsm_file.write("\n                 default: STATE <= " + self.dictionary["states"][0]+";\n")
        fsm_file.write("\n            endcase\n")
        fsm_file.write("    end\n\n")

        # Output Logic
        # s0, s1,s2,s3... 
        print(self.dictionary["s_outputs"])
        print(self.dictionary["s_outputs_conditions"])
        print(self.dictionary["s_outputs_states"])
        #----------s_output_states index: [1,2,2]-----------
        out_index = []
        l = 1
        for j in range(0,len(self.dictionary["s_outputs_states"])):
            if(j!= len(self.dictionary["s_outputs_states"])-1):
                if(self.dictionary["s_outputs_states"][j] == self.dictionary["s_outputs_states"][j+1]):
                    l = l + 1
                else:
                    out_index.append(l)
                    l = 1
            else:
                out_index.append(l)

        print(out_index)

        fsm_file.write("\n    always @(STATE) begin \n" )
        fsm_file.write("        case(STATE)\n" )

        
        i = 0   #index for index_states
        j = 0   #index for PS (0,4)
        k = 0   #Index iterations number
        l = 0   #Index for NS and Stim (0,8)

        while (j < len(self.dictionary["s_outputs_states"])):
            fsm_file.write("\n                "+self.dictionary["s_outputs_states"][j]+":")
            for k in range(0,out_index[i]):
                if(k==0):
                    if(self.dictionary["s_outputs_conditions"][l]!="-"):    
                        fsm_file.write("\n                    if("+self.dictionary["s_outputs_conditions"][l]+")")
                        fsm_file.write("\n                      begin")
                        fsm_file.write("\n                        " + self.dictionary["s_outputs"][l] + ";")
                        fsm_file.write("\n                      end\n")
                    else:
                        fsm_file.write("\n                      begin")
                        fsm_file.write("\n                       " + self.dictionary["s_outputs"][l] + ";")
                        fsm_file.write("\n                      end")
                else:
                    if(self.dictionary["s_outputs_conditions"][l]!="-"):    
                        fsm_file.write("\n                    else if("+self.dictionary["s_outputs_conditions"][l]+")")
                        fsm_file.write("\n                      begin")
                        fsm_file.write("\n                        " + self.dictionary["s_outputs"][l]+";")
                        fsm_file.write("\n                      end\n")
                    else:
                        fsm_file.write("\n                    else ")
                        fsm_file.write("\n                      begin")
                        fsm_file.write("\n                        " + self.dictionary["s_outputs"][l]+";")
                        fsm_file.write("\n                      end")
                l += 1
            j += out_index[i]
            i += 1
    
        fsm_file.write("\n        endcase\n")
        fsm_file.write("    end\n" )
        fsm_file.write("endmodule" )
            
if __name__ == "__main__":
    fsm1 = FSM() 
    fsm1.fsm_file = "Mealy.txt"
    fsm1.append_data()
    fsm1.write_FSM()
    
