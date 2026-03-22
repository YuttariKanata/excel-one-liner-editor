import re

def extract_excel_functions(input_file, output_file):
    functions = []
    
    with open(input_file, 'r', encoding='utf-8') as f:
        for line in f:
            if not line.strip():
                continue
            
            # 最初のカンマより前を取得
            first_part = line.split(',')[0].strip()
            
            # (2013) などの付随情報を削除し、アルファベットとアンダースコアのみ残す
            # 正規表現で「A-Z」の連続部分だけを抽出
            clean_name = re.match(r'^[A-Z0-9_\.]+', first_part.upper())
            
            if clean_name:
                name = clean_name.group(0)
                # 念のため、短すぎるものやノイズを除外
                if len(name) > 0:
                    functions.append(name)

    # 重複を除去してソート
    functions = sorted(list(set(functions)))

    with open(output_file, 'w', encoding='utf-8') as f:
        for func in functions:
            f.write(func + '\n')

    print(f"抽出完了: {len(functions)} 個の関数が見つかりました。 -> {output_file}")

if __name__ == "__main__":
    extract_excel_functions('fnclist.csv', 'functions.txt')