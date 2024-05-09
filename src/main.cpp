#include<bits/stdc++.h>
using namespace std;

short reg[32];
char operations[64];
short operands[64][3];
string labels[64];
int status[64];
int iss[64];
int stExec[64];
int fnExec[64];
int writeCycles[64];
int reserve[64];
int results[64];
deque<pair<int,int>> stores;
deque<pair<int,int>> loads;
map<string, int> lab;
int durations[8], stations[8];
short mem[65536];
short registerStat[32];
int ipc = 0;
int cycle = 0;
int branches = 0, taken = 0;
struct RS {
    short Qj=-1, Qk=-1, Vj, Vk, A, dur;
    bool busy = 0;
};
RS st[12];

void checkR(int src1, int src2, int des, int reserve) {
    if (registerStat[src1]>=0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    if (registerStat[src2]>=0)
        st[reserve].Qk = registerStat[src2];
    else {
        st[reserve].Vk = reg[src2];
        st[reserve].Qk = -1;
    }
    st[reserve].busy = 1;
    registerStat[des] = reserve;
}


void checkNeg(int src1, int des, int reserve) {
    if (registerStat[src1] >= 0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    st[reserve].busy = 1;
    registerStat[des] = reserve;
}

void checkJal(int reserve) {
    st[reserve].busy = 1;
    registerStat[1] = reserve;
}

void checkRet(int reserve) {
    if (registerStat[1] >= 0)
        st[reserve].Qj = registerStat[1];
    else {
        st[reserve].Vj = reg[1];
        st[reserve].Qj = -1;
    }
    st[reserve].busy = 1;
}

void checkLoad(int src1, int Imm, int des, int reserve) {
    if (registerStat[src1]>=0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    st[reserve].A = Imm;
    st[reserve].busy = 1;
    registerStat[des] = reserve;
}

void checkImm(int Imm, int des, int src1, int reserve) {
    if (registerStat[src1] >= 0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    st[reserve].A = Imm;
    st[reserve].busy = 1;
    registerStat[des] = reserve;
}

void checkStore(int Imm, int src1, int src2, int reserve) {
    if (registerStat[src1]>=0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    if (registerStat[src2]>=0)
        st[reserve].Qk = registerStat[src2];
    else {
        st[reserve].Vk = reg[src2];
        st[reserve].Qk = -1;
    }
    st[reserve].A = Imm;
    st[reserve].busy = 1;
}

void checkBeq(int src1, int src2, int reserve) {
    if (registerStat[src1] >= 0)
        st[reserve].Qj = registerStat[src1];
    else {
        st[reserve].Vj = reg[src1];
        st[reserve].Qj = -1;
    }
    if (registerStat[src2] >= 0)
        st[reserve].Qk = registerStat[src2];
    else {
        st[reserve].Vk = reg[src2];
        st[reserve].Qk = -1;
    }
    st[reserve].busy = 1;
}

int issue(int pc, int timer) {
    int reserve = -1;
    if (operations[pc] == 'A') {
        for (int i = stations[3]; i < stations[4]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkR(operands[pc][1], operands[pc][2], operands[pc][0], reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[4];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'N') {
        for (int i = stations[5]; i < stations[6]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkR(operands[pc][1], operands[pc][2], operands[pc][0], reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[6];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'M') {
        for (int i = stations[6]; i < stations[7]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkR(operands[pc][1], operands[pc][2], operands[pc][0], reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[7];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == '~') {
        for (int i = stations[4]; i < stations[5]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkNeg(operands[pc][1], operands[pc][0], reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[5];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'I') {
        for (int i = 6; i < 9; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkImm(operands[pc][2], operands[pc][0], operands[pc][1], reserve);
        iss[pc] = timer;
        st[reserve].dur = 2;
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'L') {
        for (int i = 0; i < stations[0]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkLoad(operands[pc][2], operands[pc][1], operands[pc][0], reserve);
        iss[pc] = timer;
        loads.push_back({ timer,st[reserve].A + st[reserve].Vj});
        st[reserve].dur = durations[0];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'S') {
        for (int i = stations[0]; i < stations[1]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkStore(operands[pc][1], operands[pc][2], operands[pc][0], reserve);
        iss[pc] = timer;
        stores.push_back({ timer,st[reserve].A + st[reserve].Vj });
        st[reserve].dur = durations[1];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'J') {
        for (int i = stations[2]; i < stations[3]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkJal(reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[3];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'R') {
        for (int i = stations[2]; i < stations[3]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkRet(reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[3];
        status[pc] = 1;
        timer++;
        return reserve;
    }
    else if (operations[pc] == 'B') {
        for (int i = stations[1]; i < stations[2]; i++)
            if (!(st[i].busy)) {
                reserve = i;
                break;
            }
        if (reserve == -1)
            return -1;
        checkBeq(operands[pc][0], operands[pc][1], reserve);
        iss[pc] = timer;
        st[reserve].dur = durations[2];
        status[pc] = 1;
        timer++;
        branches++;
        return reserve;
    }
}

int exec(int pc, int reserve, int timer) {
    int result = -1;
    if (operations[pc] == 'A') {
        if (st[reserve].Qj == -1 && st[reserve].Qk == -1) {
            result = st[reserve].Vj + st[reserve].Vk;
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'N') {
        if (st[reserve].Qj == -1 && st[reserve].Qk == -1) {
            result = ~(st[reserve].Vj | st[reserve].Vk);
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'M') {
        if (st[reserve].Qj == -1 && st[reserve].Qk == -1) {
            result = st[reserve].Vj * st[reserve].Vk;
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == '~') {
        if (st[reserve].Qj == -1) {
            result = -1*st[reserve].Vj;
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'I') {
        if (st[reserve].Qj == -1 && st[reserve].Qk == -1) {
            result = st[reserve].Vj + st[reserve].A;
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'L') {
        if (st[reserve].Qj == -1) {
            int dummy = st[reserve].Vj + st[reserve].A;
            for (int i = 0; i < stores.size(); i++)
                if (dummy == stores[i].second && iss[pc] > stores[i].first) {
                    st[reserve].dur = 2;
                    return -1;
                }
            st[reserve].A= dummy;
            stExec[pc] = timer++;
            status[pc] = 2;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            result = mem[st[reserve].A];
        }
    }
    else if (operations[pc] == 'S') {
        if (st[reserve].Qj == -1) {
            int dummy = st[reserve].Vj + st[reserve].A;
            for (int i = 0; i < stores.size(); i++)
                if (dummy == stores[i].second && iss[pc] > stores[i].first) {
                    st[reserve].dur = 1;
                    return -1;
                }
            for (int i = 0; i < loads.size(); i++)
                if (dummy == loads[i].second && iss[pc] > loads[i].first) {
                    st[reserve].dur = 1;
                    return -1;
                }
            st[reserve].A = dummy;
            stExec[pc] = timer++;
            result = st[reserve].Vk;
            fnExec[pc] = stExec[pc] + st[reserve].dur-1;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'J') {
        stExec[pc] = timer++;
        fnExec[pc] = stExec[pc] + st[reserve].dur - 1;
        result = lab[labels[pc]] - pc + 1;
        status[pc] = 2;
    }
    else if (operations[pc] == 'R') {
        if (st[reserve].Qj == -1) {
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur - 1;
            result = st[reserve].Vj +1 ;
            status[pc] = 2;
        }
    }
    else if (operations[pc] == 'B') {
        if (st[reserve].Qj == -1 && st[reserve].Qk == -1) {
            stExec[pc] = timer++;
            fnExec[pc] = stExec[pc] + st[reserve].dur - 1;
            result = (st[reserve].Vj == st[reserve].Vk)? lab[labels[pc]] - pc + 1 :0;
            if (result != 0) taken++;
            status[pc] = 2;
        }
    }
    return result;
}

void wb(int pc, int result, int reserve, int timer) {
    if (fnExec[pc]+1 <= timer) {
        st[reserve].busy = false;
        for (int i = 0; i < 32; i++)
        {
            if (registerStat[i] == reserve) {
                reg[i] = result;
                registerStat[i] = -1;
            }
        }
        for (int i = 0; i < 12; i++)
        {
            if (st[i].Qj == reserve) {
                st[i].Vj = result;
                st[i].Qj = -1;
            }
            if (st[i].Qk == reserve) {
                st[i].Vk = result;
                st[i].Qk = -1;
            }
        }
        if (operations[pc] == 'S') {
            mem[st[reserve].A] = result;
            stores.pop_front();
        }
        if (operations[pc] == 'L') {
            loads.pop_front();
        }
        if (operations[pc] == 'J') {
            reg[1] = pc;
            ipc = pc + result;

        }
        if (operations[pc] == 'R')
            ipc = result;
        if (operations[pc] == 'B')
            ipc = pc+ result;
        writeCycles[pc] = timer++;
        status[pc] = 3;
    }
}

int main() {
    fstream data;
    string comm;
    ipc = 0;
    data.open("Test.txt");
    if (!data.is_open())
        cout << "Error in opening data file";
    else
    {
        int address, value;
        while (!data.eof())
        {
            getline(data, comm);
            address = stoi(comm.substr(0, comm.find(' ')));
            value = stoi(comm.substr(comm.find(' ') + 1, comm.find('\n')));
            mem[address] = value;
        }
    }
    data.close();
    int inst_no;
    cout << "Enter number of reservations stations for LW, SW, BEQ, JAL/RET, ADD/ADDI, NEG, NOR, MUL in that order\n";
    cin >> stations[0] >> stations[1] >> stations[2] >> stations[3] >> stations[4] >> stations[5] >> stations[6] >> stations[7];
    for (int i = 1; i < 8; i++)
    {
        stations[i] = stations[i] + stations[i - 1];
    }
    cout << "Enter number of cycles needed for LW, SW, BEQ, JAL/RET, ADD/ADDI, NEG, NOR, MUL in that order\n";
    cin >> durations[0] >> durations[1] >> durations[2] >> durations[3] >> durations[4] >> durations[5] >> durations[6] >> durations[7];
    cout << "Enter number of instructions\n";
    cin >> inst_no;
    string choice;
    memset(registerStat, -1, sizeof(registerStat));
    for (int pc = 0; pc < inst_no; pc++) {
        status[pc] = 0;
        cout << "Please enter instruction " << pc + 1 << "\nEnter A for ADD, Enter I for ADDI, Enter N for NOR, Enter ~ for NEG, Enter M for MUL";
        cout << ", Enter B for BEQ, Enter L for LW, Enter S for SW, Enter J for JAL, Enter R for RET, Any other inputs will be labels\n";
        cin >> choice;
        if (choice == "A" || choice == "N" || choice == "M") {
            cout << "You have selected an R format operation. Please enter register numbers in this format rd, rs1, rs1\n";
            operations[pc] = choice[0];
            cin >> operands[pc][0] >> operands[pc][1] >> operands[pc][2];
        }
        else if (choice == "I") {
            cout << "You have selected an ADDI operation. Please enter register numbers in this format rd, rs1, Imm\n";
            operations[pc] = choice[0];
            cin >> operands[pc][0] >> operands[pc][1] >> operands[pc][2];
        }
        else if (choice == "~") {
            cout << "You have selected a NEG format operation. Please enter register numbers in this format rd, rs\n";
            operations[pc] = choice[0];
            cin >> operands[pc][0] >> operands[pc][1];
        }
        else if (choice == "L" || choice == "S") {
            cout << "You have selected a Memory accessing operation. Please enter register numbers in this format rs1 or rd, Imm, rs2\n";
            operations[pc] = choice[0];
            cin >> operands[pc][0] >> operands[pc][1] >> operands[pc][2];
        }
        else if (choice == "B") {
            cout << "You have selected a BEQ operation. Please enter register numbers in this format rs1, rs2, Label\n";
            operations[pc] = choice[0];
            cin >> operands[pc][0] >> operands[pc][1] >> labels[pc];
        }
        else if (choice == "J") {
            cout << "You have selected a JAL operation. Please enter register numbers in this format Label\n";
            operations[pc] = choice[0];
            cin >> labels[pc];
        }
        else if (choice == "R") {
            cout << "You have selected a RET operation. Please enter register numbers in this format rs1, rs2, Label\n";
            operations[pc] = choice[0];
        }
        else {
            lab[choice] = pc;
        }
    }
    cout << "Instruction \t| Issue \t| Start Exec. \t| Finish Exec. \t| Written Back\n";
    ipc = 0;
    int jumpLoc = -1;
    while (ipc < inst_no*10) {
        int timer = cycle;
        bool flagIssue = true, flagWrite = true;
        for (int i = 0; i <= inst_no; i++) {
            if (status[i] == 1 && (jumpLoc==-1|| i>=jumpLoc)) {
                results[i] = exec(i, reserve[i], timer);
                //if (i == jumpLoc)
                //    jumpLoc = -1;
            }
        }
        for (int i = 0; i <= inst_no; i++) {
            if (status[i] == 0 && flagIssue && operations[i] != '\0') {
                reserve[i] = issue(i, timer);
                flagIssue = false;
            }

        }
        for (int i = 0; i <= inst_no; i++) {
            if (status[i] == 2 && flagWrite) {
                wb(i, results[i], reserve[i], timer);
                if (status[i] == 3) {
                    //status[i] = 1;
                    if (operations[i] == 'J'|| operations[i]=='R' || operations[i]=='B') {
                        jumpLoc = ipc;
                        /*for (int j = jumpLoc; j < inst_no; j++)
                            status[j] = 0;*/
                    }
                    flagWrite = false;
                }
            }
        }
        cycle++;
        ipc++;
    }
    int numberCycles = 0;
    for (int i = 0; i < inst_no; i++) {
        cout << i << ". " << operations[i] << ' ' << operands[i][0] << ' ' << operands[i][1] << ' ' << operands[i][2] << "\t|\t" << iss[i] << "\t|\t" << stExec[i] << "\t|\t" << fnExec[i] << "\t|\t" << writeCycles[i] << endl;
        numberCycles = max(numberCycles, writeCycles[i]);
    }
    cout << "Total number of cycles: " << numberCycles << endl;
    cout << "IPC: " << float(inst_no) / numberCycles << endl;
    cout << "Branch misprediction percentage: " << (branches)?(100*float(taken)/branches):0 << '%';

    return 0;
}


















