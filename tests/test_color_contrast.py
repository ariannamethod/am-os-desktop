def count_contrast(a, b):
    def lum(c):
        def channel(v):
            v /= 255.0
            return v / 12.92 if v <= 0.03928 else ((v + 0.055) / 1.055) ** 2.4

        r, g, b = c
        return channel(r) * 0.2126 + channel(g) * 0.7152 + channel(b) * 0.0722

    l1 = lum(a)
    l2 = lum(b)
    brightest = max(l1, l2)
    darkest = min(l1, l2)
    return (brightest + 0.05) / (darkest + 0.05)


def is_acceptable(a, b, threshold=4.5):
    return count_contrast(a, b) >= threshold


def test_high_contrast():
    assert is_acceptable((0, 0, 0), (255, 255, 255))


def test_low_contrast():
    assert not is_acceptable((120, 120, 120), (130, 130, 130))
