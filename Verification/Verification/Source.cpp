#include<iostream>
#include<fstream>
#include<sstream>
#include <random>

#define link_to_axon_spike "C:/Users/hoang/Desktop/Group03/Verification/Data/axon_spikes.txt"
#define link_to_CSRAM_data "C:/Users/hoang/Desktop/Group03/Verification/Data/CSRAM_data.txt"
#define link_to_neuron_instruction "C:/Users/hoang/Desktop/Group03/Verification/Data/tc_000.mem"
#define link_to_output "C:/Users/hoang/Desktop/Group03/Verification/Data/output_soft.txt"
#define link_to_many_ticks_axon_spike "C:/Users/hoang/Desktop/Group03/Verification/Data/many_tick_axon_spikes.txt"

using namespace std;
////////////////////////////////////các hàm xử lý dữ liệu////////////////////////////////////
int random(int initial, int last) {
    random_device rd;
    mt19937_64 rng(rd());
    std::uniform_int_distribution<int> distribution(initial, last);
    return distribution(rng);  // Use rng as a generator
}
int binTwosComplementToSignedDecimal(string binary) {
    int significantBits = binary.size();
    int power = pow(2, significantBits - 1);
    int sum = 0;
    int i;

    for (i = 0; i < binary.size(); ++i) {
        if (i == 0 && binary[i] != '0') sum = power * -1;
        else sum += (binary[i] - '0') * power;//The -0 is needed
        power /= 2;
    }
    return sum;
}
int binaryToDecimal(int n) {
    int num = n;
    int dec_value = 0;

    // Initializing base value to 1, i.e 2^0
    int base = 1;

    int temp = num;
    while (temp) {
        int last_digit = temp % 10;
        temp = temp / 10;

        dec_value += last_digit * base;

        base = base * 2;
    }

    return dec_value;
}
string decimal_to_binary(int num_bit, int n) {
    if (n < 0) { // check if negative and alter the number
        n = pow(2, num_bit) + n;//9bit
        string result = "";
        while (n > 0) {
            result = string(1, (char)(n % 2 + 48)) + result;
            n = n / 2;
        }
        return result;
    }
    else if (n == 0) {
        string result = "";
        for (int i = 0; i < num_bit; i++) result += "0";
        return result;
    }
    else if (n > 0) {
        string result = "";
        int i = 0;
        while (i < num_bit) {
            if (n != 0) {
                result = (n % 2 == 0 ? "0" : "1") + result;
                n /= 2;
            }
            else result = "0" + result;
            i++;
        }
        return result;
    }



}
int* convertStrtoArr(string str) {
    static int arr[256];
    string temp = "";
    for (int i = 0; i < str.size(); i++) temp = temp + str[i] + " ";
    stringstream out(temp);
    int n; int i = 0;
    while (out >> n) {
        arr[i] = n; i++;
    }
    return arr;
}
int stringToInt(string str) {
    int n;
    stringstream temp(str);
    temp >> n;
    return n;
}

////////////////////////////////////Generate 1 file data////////////////////////////////////
void generateAxonSpikes() {
    srand(time(NULL));
    fstream output;
    output.open(link_to_axon_spike, ios::out);
    for (int i = 0; i < 256; i++) {
        output << random(0, 1);
    }
    output.close();
}
//Đối với nhiều tick, generate nhiều dòng axon spike, mỗi dòng tương ứng với 1 tick
void generateManyTickAxonSpikes(int num_tick) {
    srand(time(NULL));
    fstream output;
    output.open(link_to_many_ticks_axon_spike, ios::out);
    for (int k = 0; k < num_tick; k++) {
        for (int i = 0; i < 256; i++) {
            output << random(0, 1);
        }
        if (k != num_tick - 1) output << endl;
    }
    output.close();
}
void generateCSRAM() {
    srand(time(NULL));

    fstream output;
    output.open(link_to_CSRAM_data, ios::out);
    for (int k = 0; k < 256; k++) {

        for (int i = 0; i < 256; i++) {
            /* Do đây là data gen 1 cách ngẫu nhiên để test kiến trúc mà không được train từ model nên không tránh khói hiện tượng tràn bit khi tính toán trong neuron block
            để hạn chế hiện tượng này, chỉ gen synap connection cho 64 neuron đầu tiên, và 11 axon đầu tiên đối với neuron đó */

            /*Ps: data size của potential, weight ... là signed 9 bit, mà neuron block tính toán bằng cách duyệt hết tất cả các axon rồi mới xét đến leaky potential. Khi có quá
            nhiều synap connetion, weight quá lớn (xét positive, negative tương tự)... thì sẽ xảy ra hiện tượng integrated potential vượt lên qua khoảng 8 bit -> giả sử là
            9 bit: 100000000 ~ 256. Tuy nhiên do là signed nên kiến trúc sẽ hiểu thành -256 -> sai số. Qua thực nghiệm, người ta đã khảo sát được khi train các model khoảng
            9 bit này là vùng an toàn để không xảy ra trường hợp này.*/

            if (i >= 245 && k < 64) output << random(0, 1);  //synap cho neuron k 
            else output << 0;
        } //do nếu có quá nhiều synap với axon spike => quá nhiều process spike, thì cái integrated_potential nó cứ increase cho đến cực hạn, mà trong kiến trúc mình khai báo nó dạng reg signed nên lúc này mô phỏng sẽ coi nó là số âm (số bù 2) dẫn đến ra khác software

        output << decimal_to_binary(9, random(-1, 1));  //current potential cho neuron k
        output << decimal_to_binary(9, random(-1, 1));  //reset potential cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //weight 0 cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //weight 1 cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //weight 2 cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //weight 3 cho neuron k
        output << decimal_to_binary(9, random(-1, 1));  //leak cho neuron k
        output << decimal_to_binary(9, random(1, 5));   //positive threshold cho neuron k
        output << decimal_to_binary(9, random(-5, 0));  //negative threshold cho neuron k
        output << decimal_to_binary(1, random(0, 1));   //reset mode cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //dx cho neuron k
        output << decimal_to_binary(9, random(-2, 2));  //dy cho neuron k
        output << decimal_to_binary(8, random(0, 256)); //axon destination cho neuron k
        output << decimal_to_binary(4, random(0, 16));  //delivery tick cho neuron k
        if (k != 255) output << endl;
    }
    output.close();
}


////////////////////////////////////các hàm lấy data từ các file đã generate////////////////////////////////////
int* AxonSpikes(string link = link_to_axon_spike) {
    string data;
    int* axon_spikes = new int[256];
    fstream file;
    file.open(link, ios::in);
    getline(file, data);
    for (int i = 0; i < 256; i++) {
        axon_spikes[i] = int(data[i]) - 48;
    }
    file.close();
    return axon_spikes;
}
int** ManyAxonSpikes(int num_tick, string link = link_to_many_ticks_axon_spike) {
    int** ManyAxonSpikes;
    ManyAxonSpikes = new int* [num_tick];
    fstream file;
    file.open(link, ios::in);
    for (int k = 0; k < num_tick; k++) {
        ManyAxonSpikes[k] = new int[256];
        string data;
        getline(file, data);
        for (int i = 0; i < 256; i++) {
            ManyAxonSpikes[k][i] = int(data[i]) - 48;
        }
    }
    file.close();
    return ManyAxonSpikes;
}
int* NeuronInstructions(string link = link_to_neuron_instruction) {
    int* out = new int[256];
    fstream file;
    file.open(link, ios::in);
    string data;
    int n;
    for (int i = 0; i < 256; i++) {
        getline(file, data);
        stringstream temp(data);
        while (temp >> n) {
            out[i] = binaryToDecimal(n);
        }
    }
    file.close();
    return out;
}
string* CSRAM(string link) {
    string* out = new string[256];
    fstream file;
    file.open(link, ios::in);
    string data;
    for (int i = 0; i < 256; i++) {
        getline(file, data);
        out[i] = data;
    }
    file.close();
    return out;
}
int** Synaptic(string link = link_to_CSRAM_data) {
    int** Synaptic; //tạo ra một vector 2 chiều, 1 chiều là số lượng neuron, 1 chiều là số lượng axon
    Synaptic = new int* [256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) { //duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        Synaptic[i] = new int[256];
        string str = CSRAM_data[i].substr(0, 256); //lấy 256 phần tử đầu tiên trong 1 dòng của file csram
        int* temp = convertStrtoArr(str);   //chuyển toàn bộ string này thành vector int
        copy(temp, temp + 256, Synaptic[i]);                  //gán vector này vào vector 2 chiều

    }
    delete[] CSRAM_data;
    return Synaptic;
}
int* CurrentPotential(string link = link_to_CSRAM_data) {
    string* CSRAM_data = CSRAM(link);
    int* CurrentPotential = new int[256];
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(256, 9); //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 256
        CurrentPotential[i] = binTwosComplementToSignedDecimal(str);
    }
    delete[] CSRAM_data;
    return CurrentPotential;
}
int* ResetPotential(string link = link_to_CSRAM_data) {
    string* CSRAM_data = CSRAM(link);
    int* ResetPotential = new int[256];
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(265, 9); //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 265
        ResetPotential[i] = binTwosComplementToSignedDecimal(str);
    }
    delete[] CSRAM_data;
    return ResetPotential;
}
int** Weights(string link = link_to_CSRAM_data) {
    int** Weights; //tạo 1 vector 2 chiều tương ứng với 256 neuron và 4 loại weight
    Weights = new int* [256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        Weights[i] = new int[4];
        string str = CSRAM_data[i].substr(274, 36);  //lấy 36 phần từ tiếp theo, bắt đầu từ index thứ 274
        Weights[i][0] = binTwosComplementToSignedDecimal(str.substr(0, 9));
        Weights[i][1] = binTwosComplementToSignedDecimal(str.substr(9, 9));
        Weights[i][2] = binTwosComplementToSignedDecimal(str.substr(18, 9));
        Weights[i][3] = binTwosComplementToSignedDecimal(str.substr(27, 9));
    }
    delete[] CSRAM_data;
    return Weights;
}
int* Leak(string link = link_to_CSRAM_data) {
    int* Leak = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(310, 9);  //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 310
        Leak[i] = binTwosComplementToSignedDecimal(str);

    }
    delete[] CSRAM_data;
    return Leak;
}
int* PositiveThreshold(string link = link_to_CSRAM_data) {
    int* PositiveThreshold = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(319, 9);  //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 319
        PositiveThreshold[i] = binTwosComplementToSignedDecimal(str);

    }
    delete[] CSRAM_data;
    return PositiveThreshold;
}
int* NegativeThreshold(string link = link_to_CSRAM_data) {
    int* NegativeThreshold = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(328, 9);  //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 328
        NegativeThreshold[i] = binTwosComplementToSignedDecimal(str);

    }
    delete[] CSRAM_data;
    return NegativeThreshold;
}
int* ResetMode(string link = link_to_CSRAM_data) {
    int* ResetMode = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(337, 1);  //lấy 1 phần từ tiếp theo, bit 337
        ResetMode[i] = stringToInt(str);

    }
    delete[] CSRAM_data;
    return ResetMode;
}
int* dx(string link = link_to_CSRAM_data) {
    int* dx = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(338, 9);  //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 338
        dx[i] = binTwosComplementToSignedDecimal(str);

    }
    delete[] CSRAM_data;
    return dx;
}
int* dy(string link = link_to_CSRAM_data) {
    int* dy = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(347, 9);  //lấy 9 phần từ tiếp theo, bắt đầu từ index thứ 347 
        dy[i] = binTwosComplementToSignedDecimal(str);

    }
    delete[] CSRAM_data;
    return dy;
}
int* AxonDestination(string link = link_to_CSRAM_data) {
    int* AxonDestination = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(356, 8);  //lấy 8 phần từ tiếp theo, bắt đầu từ index thứ 356 
        AxonDestination[i] = binaryToDecimal(stringToInt(str));

    }
    delete[] CSRAM_data;
    return AxonDestination;
}
int* DeliveryTick(string link = link_to_CSRAM_data) {
    int* DeliveryTick = new int[256];
    string* CSRAM_data = CSRAM(link);
    for (int i = 0; i < 256; i++) {//duyệt từng neuron, tức là soát từng hàng 1 trong cái data từ CSRAM
        string str = CSRAM_data[i].substr(364, 4);  //lấy 4 phần từ tiếp theo, bắt đầu từ index thứ 364 
        DeliveryTick[i] = binaryToDecimal(stringToInt(str));

    }
    delete[] CSRAM_data;
    return DeliveryTick;
}
////////////////////////////////////Giả lập đối với 1 file////////////////////////////////////
//Đối với 1 tick
void NeuronGrid(string link_spike = link_to_axon_spike, string link_CSRAM = link_to_CSRAM_data, string link_out = link_to_output) {
    int* current_potential = CurrentPotential(link_CSRAM);
    int** weights = Weights(link_CSRAM);
    int* neuron_instructions = NeuronInstructions();
    int** synaptic = Synaptic(link_CSRAM);
    int* axon_spikes = AxonSpikes(link_spike);
    int* leak = Leak(link_CSRAM);
    int* reset_mode = ResetMode(link_CSRAM);
    int* positive_threshold = PositiveThreshold(link_CSRAM);
    int* negative_threshold = NegativeThreshold(link_CSRAM);
    int* reset_potential = ResetPotential(link_CSRAM);

    string spike_out = "";

    //int num_spike_out = 0;
    for (int i = 0; i <= 255; i++) {//duyệt từng neuron

        int integrated_potential = current_potential[i];
        for (int j = 255; j >= 0; j--) {//duyệt từng axon

            int weight = weights[i][neuron_instructions[j]]; //Weights tại neuron thứ i với neuron instruction của axon thứ j
            int reg_en = (synaptic[i][j] & axon_spikes[j]); //Synaptic tại neuron thứ i với axon thứ j và xem axon thứ j có spike hay k
            integrated_potential = reg_en ? integrated_potential + weight : integrated_potential;
            //if (integrated_potential > 511) integrated_potential = integrated_potential - 512; //do trong kiến trúc thằng này chỉ có 9 bit nên khi vượt quá 511 sẽ quay về 0

        }
        //sau khi duyệt hết 256 axon

        integrated_potential += leak[i];//Leak tại neuron i


        //if (integrated_potential > 511) integrated_potential = integrated_potential - 512; //do trong kiến trúc thằng này chỉ có 9 bit nên khi vượt quá 511 sẽ quay về 0

        //check xem có bắn spike hay k
        if (integrated_potential >= positive_threshold[i]) {
            //num_spike_out++;
            //cout << "\nspike at neuron " << i;
            spike_out = '1' + spike_out;
        }
        else spike_out = '0' + spike_out;

        //reset
        if (reset_mode[i] == 0) {//absolute reset
            if (integrated_potential >= positive_threshold[i]) integrated_potential = reset_potential[i];
            else if (integrated_potential < negative_threshold[i]) integrated_potential = -reset_potential[i];
            else integrated_potential = integrated_potential;
        }
        else {//linear reset
            if (integrated_potential >= positive_threshold[i]) integrated_potential = integrated_potential - positive_threshold[i];
            else if (integrated_potential < negative_threshold[i]) integrated_potential = integrated_potential + negative_threshold[i];
            else integrated_potential = integrated_potential;
        }
        current_potential[i] = integrated_potential;

    }
    fstream file;
    file.open(link_out, ios::out);
    file << spike_out;
    file.close();
    //////////////////////////////////////////////////////////////////////////
    for (int h = 0; h < 256; h++)
    {
        delete[] synaptic[h];
        delete[] weights[h];
    }
    delete[] synaptic;
    delete[] weights;
    delete[] axon_spikes;
    delete[] neuron_instructions;
    delete[] current_potential;
    delete[] leak;
    delete[] reset_mode;
    delete[] positive_threshold;
    delete[] negative_threshold;
    delete[] reset_potential;
}
//chạy khối neuron grid trong nhiều tick
void ManyTick(int num_tick, string link_spike = link_to_many_ticks_axon_spike, string link_CSRAM = link_to_CSRAM_data, string link_out = link_to_output) {
    int* current_potential = CurrentPotential(link_CSRAM);
    int** weights = Weights(link_CSRAM);
    int* neuron_instructions = NeuronInstructions();
    int** synaptic = Synaptic(link_CSRAM);
    int** many_axon_spikes = ManyAxonSpikes(num_tick, link_spike);
    int* leak = Leak(link_CSRAM);
    int* reset_mode = ResetMode(link_CSRAM);
    int* positive_threshold = PositiveThreshold(link_CSRAM);
    int* negative_threshold = NegativeThreshold(link_CSRAM);
    int* reset_potential = ResetPotential(link_CSRAM);



    fstream file;
    file.open(link_out, ios::out);
    for (int k = 0; k < num_tick; k++) {
        string spike_out = "";
        for (int i = 0; i <= 255; i++) {//duyệt từng neuron
            int integrated_potential = current_potential[i];
            for (int j = 255; j >= 0; j--) {//duyệt từng axon

                int weight = weights[i][neuron_instructions[j]]; //Weights tại neuron thứ i với neuron instruction của axon thứ j
                int reg_en = (synaptic[i][j] & many_axon_spikes[k][j]); //Synaptic tại neuron thứ i với axon thứ j và xem axon thứ j tại thời điểm tick thứ k có spike hay k
                integrated_potential = reg_en ? integrated_potential + weight : integrated_potential;
            }

            //sau khi duyệt hết 256 axon

            integrated_potential += leak[i];//Leak tại neuron i


            //check xem có bắn spike hay k
            if (integrated_potential >= positive_threshold[i]) {
                //num_spike_out++;
                spike_out = '1' + spike_out;
            }
            else spike_out = '0' + spike_out;

            //reset
            if (reset_mode[i] == 0) {//absolute reset
                if (integrated_potential >= positive_threshold[i]) integrated_potential = reset_potential[i];
                else if (integrated_potential < negative_threshold[i]) integrated_potential = -reset_potential[i];
                else integrated_potential = integrated_potential;
            }
            else {//linear reset
                if (integrated_potential >= positive_threshold[i]) integrated_potential = integrated_potential - positive_threshold[i];
                else if (integrated_potential < negative_threshold[i]) integrated_potential = integrated_potential + negative_threshold[i];
                else integrated_potential = integrated_potential;
            }
            current_potential[i] = integrated_potential;
        }
        file << spike_out;
        if (k != num_tick - 1) file << "\n";
    }


    file.close();


    //////////////////////////////////////////////////////////////////////////
    for (int h = 0; h < 256; h++) {
        delete[] synaptic[h];
        delete[] weights[h];
    }
    for (int h = 0; h < num_tick; h++) delete[] many_axon_spikes[h];
    delete[] many_axon_spikes;
    delete[] synaptic;
    delete[] weights;
    delete[] neuron_instructions;
    delete[] current_potential;
    delete[] leak;
    delete[] reset_mode;
    delete[] positive_threshold;
    delete[] negative_threshold;
    delete[] reset_potential;
}

int main() {
    ///////////Generate////////////
    //generateCSRAM(); 
    //generateAxonSpikes();                     //1 tick                       
    //generateManyTickAxonSpikes(16);           //16 tick
    //////////Simulation//////////
    //NeuronGrid();                           //1 tick   
    ManyTick(16);                           //16 tick
    return 0;
}