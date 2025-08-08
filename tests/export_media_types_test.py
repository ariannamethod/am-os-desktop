import os

def test_export_schema_mentions_new_media_types():
    repo_root = os.path.dirname(os.path.dirname(__file__))
    doc_path = os.path.join(repo_root, 'docs', 'export_schema.md')
    assert os.path.exists(doc_path), 'export_schema.md missing'
    with open(doc_path, 'r', encoding='utf-8') as f:
        content = f.read().lower()
    for keyword in ('dice', 'story', 'giveaway', 'wallpaper'):
        assert keyword in content, f'{keyword} not documented'
