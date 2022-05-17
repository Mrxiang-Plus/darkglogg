adb shell dumpsys media.camera | egrep "Stream[.*]|Consumer name|Dims:" > camera_stream.txt
glogg camera_stream.txt
