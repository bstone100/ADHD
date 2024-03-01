#ifdef __cplusplus
extern "C" {
#endif

void setOutputFileName(const char* path);
void startRecording();
void stopRecording();
float getCurrentLevel();
bool isRecording();

#ifdef __cplusplus
}
#endif
