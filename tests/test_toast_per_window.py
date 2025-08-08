class DummyToast:
    def __init__(self):
        self.hidden = False
    def hideAnimated(self):
        self.hidden = True


def hide_existing(shown, window_id):
    toast = shown.get(window_id)
    if toast:
        toast.hideAnimated()
        shown.pop(window_id, None)


def test_toasts_are_isolated():
    shown = {}
    w1, w2 = 1, 2
    t1, t2 = DummyToast(), DummyToast()
    shown[w1] = t1
    shown[w2] = t2

    hide_existing(shown, w1)

    assert t1.hidden
    assert not t2.hidden
    assert w1 not in shown and w2 in shown
