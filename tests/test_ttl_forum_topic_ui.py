class ForumTopic:
    def __init__(self):
        self.ttl = 0

    def set_ttl(self, ttl):
        self.ttl = ttl

    def clear_ttl(self):
        self.ttl = 0


def test_set_and_clear_ttl():
    topic = ForumTopic()
    topic.set_ttl(60)
    assert topic.ttl == 60
    topic.clear_ttl()
    assert topic.ttl == 0
