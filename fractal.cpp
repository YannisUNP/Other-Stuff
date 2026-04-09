#include <iostream>
#include <fstream>
#include <cstdlib>
#include <omp.h>
using namespace std;

#define DIM 768
#define THREAD 6
struct cuComplex {
    float r;
    float i;
    cuComplex(float a, float b) : r(a), i(b) {}
    float magnitude2(void) { return r * r + i * i; }
    cuComplex operator*(const cuComplex& a) {
        return cuComplex(r * a.r - i * a.i, i * a.r + r * a.i);
    }
    cuComplex operator+(const cuComplex& a) {
        return cuComplex(r + a.r, i + a.i);
    }
};

int julia(int x, int y) {
    const float scale = 1.5;
    float jx = scale * (float)(DIM / 2 - x) / (DIM / 2);
    float jy = scale * (float)(DIM / 2 - y) / (DIM / 2);

    cuComplex c(-0.7269, 0.1889);
    cuComplex a(jx, jy);

    for (int i = 0; i < 300; i++) {
        a = a * a + c;
        if (a.magnitude2() > 1000)
            return 0;
    }
    return 1;
}

//1D rowwise parallel
void kernel_ompA(unsigned char* ptr) {
    omp_set_num_threads(THREAD);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int tthreads = omp_get_num_threads();

        int threadRows = DIM / tthreads;
        int startY = id*threadRows;
        int endY = (id + 1) * threadRows;

        for(int y = startY; y < endY; y++){
            for(int x = 0; x < DIM; x++){
                int offset = x + y * DIM;

                int juliaValue = julia(x, y);
                ptr[offset * 3 + 0] = 255 * juliaValue; // R
                ptr[offset * 3 + 1] = 0;               // G
                ptr[offset * 3 + 2] = 0;               // B
            }
        }
    }
}

//1D columnwise parallel
void kernel_ompB(unsigned char* ptr) {
    omp_set_num_threads(THREAD);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int tthreads = omp_get_num_threads();

        int threadCols = DIM / tthreads;
        int startX = id*threadCols;
        int endY = (id + 1) * threadCols;

        for(int y = 0; y < DIM; y++){
            for(int x = startX; x < endY; x++){
                int offset = x + y * DIM;

                int juliaValue = julia(x, y);
                ptr[offset * 3 + 0] = 255 * juliaValue; // R
                ptr[offset * 3 + 1] = 0;               // G
                ptr[offset * 3 + 2] = 0;               // B
            }
        }
    }
}

//2D row-block parallel
void kernel_ompC(unsigned char* ptr){
    omp_set_num_threads(THREAD);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int tthreads = omp_get_num_threads();

        int threadRows = DIM / tthreads;
        int startY = id*threadRows;
        int endY;
        if(id == tthreads -1){
            endY = DIM;
        }{
            endY = startY + threadRows;
        }

        for(int y = startY; y < endY; y++){
            for(int x = 0; x < DIM; x++){
                int offset = x + y * DIM;

                int juliaValue = julia(x,y);
                ptr[offset * 3 + 0] = 255 * juliaValue; // R
                ptr[offset * 3 + 1] = 0;               // G
                ptr[offset * 3 + 2] = 0;               // B

            }
        }
    }
}

//2D column-block
void kernel_ompD(unsigned char* ptr) {
    omp_set_num_threads(THREAD);
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int tthreads = omp_get_num_threads();

        int threadCols = DIM / tthreads;
        int startX = id*threadCols;
        int endY;
        if(id == tthreads - 1){
            endY = DIM;
        }else{
            endY = startX + threadCols;
        }

        for(int y = 0; y < DIM; y++){
            for(int x = startX; x < endY; x++){
                int offset = x + y * DIM;

                int juliaValue = julia(x, y);
                ptr[offset * 3 + 0] = 255 * juliaValue; // R
                ptr[offset * 3 + 1] = 0;               // G
                ptr[offset * 3 + 2] = 0;               // B
            }
        }
    }
}

void kernel_ompE(unsigned char* ptr) {
    omp_set_num_threads(THREAD);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < DIM; y++) {
        for (int x = 0; x < DIM; x++) {
            int offset = x + y * DIM;

            int juliaValue = julia(x, y);
            ptr[offset * 3 + 0] = 255 * juliaValue; // R
            ptr[offset * 3 + 1] = 0;               // G
            ptr[offset * 3 + 2] = 0;               // B
        }
    }
}

void kernel_serial(unsigned char* ptr) {
    for (int y = 0; y < DIM; y++) {
        for (int x = 0; x < DIM; x++) {
            int offset = x + y * DIM;

            int juliaValue = julia(x, y);
            ptr[offset * 3 + 0] = 255 * juliaValue;
            ptr[offset * 3 + 1] = 0;
            ptr[offset * 3 + 2] = 0;
        }
    }
}


/* Save image as PPM */
void save_ppm(const char* filename, unsigned char* data, int width, int height) {
    ofstream file(filename, ios::binary);
    file << "P6\n" << width << " " << height << "\n255\n";
    file.write(reinterpret_cast<char*>(data), width * height * 3);
    file.close();
}

int main(void) {
    unsigned char* image_s = new unsigned char[DIM * DIM * 3];
    unsigned char* image_p = new unsigned char[DIM * DIM * 3];
    const int num_runs = 10;
    double start;

    // 1. Serial Run (Average of 10)
    double total_s = 0;
    for(int i = 0; i < num_runs; i++) {
        start = omp_get_wtime();
        kernel_serial(image_s);
        total_s += (omp_get_wtime() - start);
    }
    double avg_s = total_s / num_runs;

    // 2. Function to automate the parallel averages
    auto get_avg = [&](void (*kernel)(unsigned char*)) {
        double total = 0;
        for(int i = 0; i < num_runs; i++) {
            start = omp_get_wtime();
            kernel(image_p);
            total += (omp_get_wtime() - start);
        }
        return total / num_runs;
    };

    double avg_pA = get_avg(kernel_ompA);
    double avg_pB = get_avg(kernel_ompB);
    double avg_pC = get_avg(kernel_ompC);
    double avg_pD = get_avg(kernel_ompD);
    double avg_pE = get_avg(kernel_ompE);

    // Print Results
    cout << "Average Execution Times (over " << num_runs << " runs):" << endl;
    cout << "Serial: " << avg_s << "s" << endl;
    cout << "1D Row: " << avg_pA << "s | Speedup: " << avg_s / avg_pA << endl;
    cout << "1D Col: " << avg_pB << "s | Speedup: " << avg_s / avg_pB << endl;
    cout << "2D Row: " << avg_pC << "s | Speedup: " << avg_s / avg_pC << endl;
    cout << "2D Col: " << avg_pD << "s | Speedup: " << avg_s / avg_pD << endl;
    cout << "For Con: " << avg_pE << "s | Speedup: " << avg_s / avg_pE << endl;

    delete[] image_s;
    delete[] image_p;
    return 0;
}
