import os
import sys
import threading


class OutputGrabber(object):
    """
    Class used to grab standard output or another stream.
    """
    escape_char = "\b"

    def __init__(self, stream=None):
        self.origstream = stream
        if self.origstream is None:
            self.origstream = sys.stdout
        self.origstream_fileno = self.origstream.fileno()
        self.capturedtext = ""
        pass

    def start(self):
        """
        Start capturing the stream data.
        """
        self.capturedtext = ""
        # Save a copy of the stream:
        self.origstream_save = os.dup(self.origstream_fileno)
        self.origstream_pipe = os.pipe()
        # Replace the Original stream with our write pipe
        os.dup2(self.origstream_pipe[1], self.origstream_fileno)
        os.close(self.origstream_pipe[1])
        self.t = threading.Thread(target=self.readOutput)
        self.t.start()

    def stop(self):
        """
        Stop capturing the stream data and save the text in `capturedtext`.
        """
        os.close(self.origstream_fileno)
        self.t.join()
        
        # Clean up the pipe and restore the original stdout
        os.close(self.origstream_pipe[0])
        
        os.dup2(self.origstream_save, self.origstream_fileno)
        os.close(self.origstream_save)

    def readOutput(self):
        """
        Read the stream data (one byte at a time)
        and save the text in `capturedtext`.
        """
        while True:
            data = os.read(self.origstream_pipe[0], 1024)
            if not data:
                break
            self.capturedtext += data
