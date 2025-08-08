import unittest


class FakeMixer:
    def __init__(self):
        self._mute_listeners = []
        self._unmute_listeners = []

    def on_muted(self, callback):
        self._mute_listeners.append(callback)

    def on_unmuted(self, callback):
        self._unmute_listeners.append(callback)

    def notify_muted(self, track):
        for cb in list(self._mute_listeners):
            cb(track)

    def notify_unmuted(self, track):
        for cb in list(self._unmute_listeners):
            cb(track)


class FakeTrack:
    def __init__(self, mixer):
        self.paused = False
        self.position = 0
        mixer.on_muted(self._on_muted)
        mixer.on_unmuted(self._on_unmuted)

    def _on_muted(self, track):
        if track is self:
            self.paused = True

    def _on_unmuted(self, track):
        if track is self:
            self.paused = False


class MixerMuteTest(unittest.TestCase):
    def test_track_pauses_when_muted_externally(self):
        mixer = FakeMixer()
        track = FakeTrack(mixer)

        mixer.notify_muted(track)
        self.assertTrue(track.paused)

        mixer.notify_unmuted(track)
        self.assertFalse(track.paused)


if __name__ == "__main__":
    unittest.main()

