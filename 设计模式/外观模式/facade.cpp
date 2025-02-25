#include <iostream>

using namespace std;

class AudioStream{
public:
    void decoderAudio(){
        cout<<"Decoding audio..."<<endl;
    }    
};

class VideoStream{
public:
    void decoderVideo(){
        cout<<"Decoding video..."<<endl;
    }
};

class Screen{
public:
    void display(){
        cout<<"Displaying..."<<endl;
    }
};

// 外观类：提供一个简单的接口来操作子系统
class MediaPlayerInterface{
private:
    AudioStream audio;
    VideoStream video;
    Screen screen;
public:
    void play(){
        audio.decoderAudio();
        video.decoderVideo();
        screen.display();
    }
};

int main(){
    MediaPlayerInterface player;
    player.play();

    return 0;
}

