/*
  Bare result without any modifications: Execution time: 11325[ms]
  --------------------------------------
  Modification 1: Using future and async, with launch::async (starts immidiately, in a new thread) Execution time: 3819[ms] almost 3 times performance gain,
  i have also tried to put "result" array creation into separate thread but it seems that i slightly lose performance by doing so, presumably due to thread creation overhead.
  --------------------------------------
  Modification 2: Using adjustable amount of threads and condition variable to notify start of addPixelColor i have achieved: Execution time: 1908[ms] with 8 threads used for each part.
  --------------------------------------
  Modification 3: Using array of futures and promises making threads which call addPixelColor to wait for specific parts of the picture to be ready and
  partially update the picture, instead of waiting for two images to be completely ready gives roughly same result as modification 2. With 8 threads Execution time: 1883[ms]
*/


#include <iostream>
#include <chrono>
#include <future>
#include <mutex>
#include <condition_variable>

using namespace std;
condition_variable cv;
mutex m;
struct Pixel
{
    float red;
    float green;
    float blue;
};
/******************************/
//First modification
/*
void addPixelColors(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize)
{
    for (int i = 0; i < imageSize; i++)
    {
        result[i].red = image1[i].red + image2[i].red;
        if (result[i].red > 1.0f)
        {
            result[i].red = 1.0f;
        }

        result[i].green = image1[i].green + image2[i].green;
        if (result[i].green > 1.0f)
        {
            result[i].green = 1.0f;
        }

        result[i].blue = image1[i].blue + image2[i].blue;
        if (result[i].blue > 1.0f)
        {
            result[i].blue = 1.0f;
        }
    }
}


Pixel* createPixels(int imageSize)
{
    Pixel* image = new Pixel[imageSize];
    for (int i = 0; i < imageSize; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
    return image;
}


int main()
{
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    constexpr int imageSize = 4096 * 4096;
    future<Pixel*> image1fut = async(launch::async, &createPixels, imageSize);
    future<Pixel*> image2fut = async(launch::async, &createPixels, imageSize);
    Pixel* image1 = image1fut.get();
    Pixel* image2 = image2fut.get();
    Pixel* result = new Pixel[imageSize];

    addPixelColors(image1, image2, result, imageSize);

    chrono::steady_clock::time_point end = chrono::steady_clock::now();

    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    delete[] result;
    delete[] image2;
    delete[] image1;
}
*/
/******************************/
//Second modification
/*
void addPixelColors(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize, int blockNum, int blocks)
{
    int rem = imageSize % blocks;
    int extraPixels = 0;
    if (blockNum == blocks - 1)
        extraPixels = rem;
    for (int i = blockNum * (int)(imageSize / blocks); i < (blockNum + 1) * (int)(imageSize / blocks) + extraPixels; i++)
    {
        result[i].red = image1[i].red + image2[i].red;
        if (result[i].red > 1.0f)
        {
            result[i].red = 1.0f;
        }

        result[i].green = image1[i].green + image2[i].green;
        if (result[i].green > 1.0f)
        {
            result[i].green = 1.0f;
        }

        result[i].blue = image1[i].blue + image2[i].blue;
        if (result[i].blue > 1.0f)
        {
            result[i].blue = 1.0f;
        }
    }
}



void createPixels(Pixel *image, int imageSize, int blockNum, int blocks, bool* blocksReady)
{
    int rem = imageSize % blocks;
    int extraPixels = 0;
    if (blockNum == blocks - 1)
        extraPixels = rem;
    for (int i = blockNum * (int)(imageSize / blocks); i < (blockNum + 1) * (int)(imageSize / blocks) + extraPixels; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
    blocksReady[blockNum] = true;
    cv.notify_one();
}

int main()
{

    //Second modification
    int amount;
    cout << "Input number of threads per image: ";
    cin >> amount;
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    constexpr int imageSize = 4096 * 4096;

    bool* image1Ready = new bool[amount]();
    Pixel* image1 = new Pixel[imageSize];
    vector<thread> image1Threads;
    for (int i = 0; i < amount; i++)
    {
        image1Threads.push_back(thread(&createPixels, image1, imageSize, i, amount, image1Ready));
    }

    bool* image2Ready = new bool[amount]();
    Pixel* image2 = new Pixel[imageSize];
    vector<thread> image2Threads;
    for (int i = 0; i < amount; i++)
    {
        image2Threads.push_back(thread(&createPixels, image2, imageSize, i, amount, image2Ready));
    }
    Pixel* result = new Pixel[imageSize];
    for (int i = 0; i < amount; i++)
        image1Threads[i].join();
    for (int i = 0; i < amount; i++)
        image2Threads[i].join();
    unique_lock<mutex> lock(m);
    cv.wait(lock, [image1Ready, image2Ready, amount]() {
        for (int i = 0; i < amount; i++) {
            if (!image1Ready[i] || !image2Ready[i]) {
                return false;
            }
        }
        return true;
        });
    vector<thread> threads;
    for (int i = 0; i < amount; i++)
    {
        threads.push_back(thread(&addPixelColors, image1, image2, result, imageSize, i, amount));
    }
    for (int i = 0; i < amount; i++) {
        threads[i].join();
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    delete[] result;
    delete[] image2;
    delete[] image1;

    return 0;
    
}
*/
/******************************/
//Third modification

void addPixelColors(const Pixel* image1, const Pixel* image2, Pixel* result, int imageSize, int blockNum, int blocks, future<bool>&blockReadyImg1, future<bool>& blockReadyImg2)
{
    blockReadyImg1.get();
    blockReadyImg2.get();
    int rem = imageSize % blocks;
    int extraPixels = 0;
    if (blockNum == blocks - 1)
        extraPixels = rem;
    for (int i = blockNum * (int)(imageSize / blocks); i < (blockNum + 1) * (int)(imageSize / blocks) + extraPixels; i++)
    {
        result[i].red = image1[i].red + image2[i].red;
        if (result[i].red > 1.0f)
        {
            result[i].red = 1.0f;
        }

        result[i].green = image1[i].green + image2[i].green;
        if (result[i].green > 1.0f)
        {
            result[i].green = 1.0f;
        }

        result[i].blue = image1[i].blue + image2[i].blue;
        if (result[i].blue > 1.0f)
        {
            result[i].blue = 1.0f;
        }
    }
}



void createPixels(Pixel* image, int imageSize, int blockNum, int blocks, promise<bool> &blockReadyPromise)
{
    int rem = imageSize % blocks;
    int extraPixels = 0;
    if (blockNum == blocks - 1)
        extraPixels = rem;
    for (int i = blockNum * (int)(imageSize / blocks); i < (blockNum + 1) * (int)(imageSize / blocks) + extraPixels; i++)
    {
        image[i].red = (float(rand()) / float((RAND_MAX)));
        image[i].green = (float(rand()) / float((RAND_MAX)));
        image[i].blue = (float(rand()) / float((RAND_MAX)));
    }
    blockReadyPromise.set_value(true);
}


int main()
{
    int amount;
    cout << "Input number of threads per image: ";
    cin >> amount;
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    constexpr int imageSize = 4096 * 4096;

    vector<promise<bool>> image1Promises(amount);
    vector<promise<bool>> image2Promises(amount);
    vector<future<bool>> image1Futures;
    vector<future<bool>> image2Futures;
    for (int i = 0; i < amount; ++i) {
        image1Futures.push_back(image1Promises[i].get_future());
        image2Futures.push_back(image2Promises[i].get_future());
    }

    Pixel* image1 = new Pixel[imageSize];
    Pixel* image2 = new Pixel[imageSize];
    Pixel* result = new Pixel[imageSize];

    vector<thread> image1Threads;
    vector<thread> image2Threads;
    for (int i = 0; i < amount; ++i) {
        image1Threads.push_back(thread(&createPixels, image1, imageSize, i, amount, ref(image1Promises[i])));
        image2Threads.push_back(thread(&createPixels, image2, imageSize, i, amount, ref(image2Promises[i])));
    }

    vector<thread> colorThreads;
    for (int i = 0; i < amount; ++i) {
        colorThreads.push_back(thread(&addPixelColors, image1, image2, result, imageSize, i, amount, ref(image1Futures[i]), ref(image2Futures[i])));
    }

    for (int i = 0; i < amount; i++) {
        colorThreads[i].join();
    }

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Execution time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "[ms]\n";

    for (int i = 0; i < amount; i++) {
        image1Threads[i].join();
    }
    for (int i = 0; i < amount; i++) {
        image2Threads[i].join();
    }

    delete[] result;
    delete[] image2;
    delete[] image1;

    return 0;
}