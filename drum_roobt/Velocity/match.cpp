#include <bits/stdc++.h>
using namespace std;

struct Seg {
    double start, end;
    int drum_avg;    // 0~3
    int cymbal_avg;  // 0~3
};

/* ================== 재사용 함수들 ================== */

// 세기 파일 로드 (헤더 허용, 공백/탭/콤마 혼용)
bool loadSegments(const string& intensityFile, vector<Seg>& segs) {
    ifstream fin(intensityFile);
    if (!fin) return false;

    string line;
    bool headerDone = false;

    while (getline(fin, line)) {
        // 1) 라인 트림
        auto trim = [](const string& s) -> string {
            auto issp = [](unsigned char c){ return std::isspace(c); };
            auto b = find_if_not(s.begin(), s.end(), issp);
            auto e = find_if_not(s.rbegin(), s.rend(), issp).base();
            return (b >= e) ? string() : string(b, e);
        };
        line = trim(line);
        if (line.empty()) continue;

        // 2) 콤마→공백 치환 + 공백 파싱 (토큰화)
        vector<string> toks;
        {
            string tmp = line;
            for (char& c : tmp) if (c == ',') c = ' ';
            istringstream iss(tmp);
            string tok;
            while (iss >> tok) toks.push_back(trim(tok)); // 토큰도 트림
        }
        if (toks.size() < 4) continue;

        // 3) BOM 제거(첫 토큰에만 필요 시)
        if (!toks[0].empty()
            && (unsigned char)toks[0][0] == 0xEF
            && toks[0].size() >= 3
            && (unsigned char)toks[0][1] == 0xBB
            && (unsigned char)toks[0][2] == 0xBF) {
            toks[0].erase(0, 3);
        }

        // 4) 헤더 스킵: 앞 두 칼럼이 숫자가 아니면 헤더로 간주
        auto looksNumber = [](const string& s) {
            if (s.empty()) return false;
            char* endp = nullptr;
            (void)strtod(s.c_str(), &endp);
            return endp != s.c_str(); // 앞에서 숫자를 하나라도 읽었는지
        };
        if (!headerDone && (!looksNumber(toks[0]) || !looksNumber(toks[1]))) {
            headerDone = true;
            continue;
        }

        // 5) 파싱 + 반올림(세기 0~3 가정)
        Seg s{};
        s.start      = stod(toks[0]);
        s.end        = stod(toks[1]);
        s.drum_avg   = (int)lround(stod(toks[2]));
        s.cymbal_avg = (int)lround(stod(toks[3]));
        segs.push_back(s);
        headerDone = true;
    }
    fin.close();

    // 6) 구간 정렬
    sort(segs.begin(), segs.end(), [](const Seg& a, const Seg& b){
        if (a.start == b.start) return a.end < b.end;
        return a.start < b.start;
    });

    return true;
}

// 세기 적용: scoreIn(Δt 기반) → scoreOut(탭 구분). R/L velocity만 덮어쓰기, 나머지 컬럼 보존
bool applyIntensityToScore(const vector<Seg>& segs,
    const string& scoreIn,
    const string& scoreOut,
    bool mapTo357 = true)
{
    ifstream sin(scoreIn);
    if (!sin) return false;
    ofstream sout(scoreOut);
    if (!sout) return false;

    // ---- 로컬 유틸(전부 이 함수 안에) ----
        auto trim = [](const string& s) -> string {
        auto issp = [](unsigned char c){ return std::isspace(c); };
        auto b = find_if_not(s.begin(), s.end(), issp);
        auto e = find_if_not(s.rbegin(), s.rend(), issp).base();
        return (b >= e) ? string() : string(b, e);
    };
    size_t cursor = 0; // 전진 포인터
    auto findSeg = [&](double t) -> const Seg* {
    while (cursor < segs.size()) {
        const Seg& s = segs[cursor];
        if (t < s.start) {
            if (cursor == 0) return nullptr;
            const Seg& p = segs[cursor - 1];
            if (t >= p.start && t < p.end) return &p;
            return nullptr;
        }
        if (t >= s.start && t < s.end) return &s;
        ++cursor;
    }
        if (!segs.empty()) {
            const Seg& last = segs.back();
            if (t >= last.start && t < last.end) return &last;
        }
        return nullptr;
    };
    auto mapIntensity = [&](int base) -> int {
        if (!mapTo357) return base;            // 원시 세기 사용 옵션
        if (base <= 1) return 3;               // 0/1 -> 3
        if (base == 2) return 5;               // 2     -> 5
        return 7;                               // 3+    -> 7
    };
    // --------------------------------------

    double accumTime = 0.0;
    string line;

    while (getline(sin, line)) {
        string raw = trim(line);
        if (raw.empty()) { sout << "\n"; continue; }

        // 콤마 → 공백 치환 + 공백/탭 파싱
        vector<string> toks;
        {
            string tmp = raw;
            for (char& c : tmp) if (c == ',') c = ' ';
            istringstream iss(tmp);
            string tok;
            while (iss >> tok) toks.push_back(tok);
        }

        // 최소 5컬럼(time R_inst L_inst R_vel L_vel) 아니면 원본 그대로 출력
        if (toks.size() < 5) { sout << raw << "\n"; continue; }

        // time은 Δt로 가정(절대시간이면 아래 한 줄을 accumTime = dt; 로 변경)
        double dt;
        try { dt = stod(toks[0]); }
        catch(...) { sout << raw << "\n"; continue; }
        accumTime += dt; // ← 절대시간이면: accumTime = dt;

        // 필드 파싱
        int R_inst, L_inst, R_vel, L_vel;
        try {
        R_inst = stoi(toks[1]);
        L_inst = stoi(toks[2]);
        R_vel  = stoi(toks[3]);
        L_vel  = stoi(toks[4]);
        } catch(...) { sout << raw << "\n"; continue; }

        // 구간 찾고 세기 적용
        if (!segs.empty()) {
            if (const Seg* seg = findSeg(accumTime)) {
                auto pick = [&](int inst, int oldv) -> int {
                    if (inst == 0) return oldv; // 무음은 유지
                    int base = (inst>=1 && inst<=4) ? seg->drum_avg
                        : (inst>=5 && inst<=8) ? seg->cymbal_avg : 0;
                    return mapIntensity(base);
                };
                R_vel = pick(R_inst, R_vel);
                L_vel = pick(L_inst, L_vel);
            }
        }

        // 갱신 후 탭 구분으로 출력(추가 컬럼 보존)
        toks[3] = to_string(R_vel);
        toks[4] = to_string(L_vel);
        for (size_t i = 0; i < toks.size(); ++i) {
            if (i) sout << "\t";
            sout << toks[i];
        }
        sout << "\n";
    }
    return true;
}


/* ================== 실행부 ================== */

int main() {
    ios::sync_with_stdio(false);

    cout << "세기 파일 경로: ";
    string intensityFile, scoreIn;
    cin >> intensityFile;
    cout << "악보 파일 경로: ";
    cin >> scoreIn;

    // 출력 파일명: scoreIn에 _vel.csv
    string scoreOut;
    size_t dot = scoreIn.find_last_of('.');
    if (dot != string::npos) scoreOut = scoreIn.substr(0, dot) + "_vel.csv";
    else scoreOut = scoreIn + "_vel.csv";

    vector<Seg> segs;
    if (!loadSegments(intensityFile, segs)) {
        cerr << "[ERROR] 세기 파일 열기/파싱 실패: " << intensityFile << "\n";
        return 1;
    }
    if (!applyIntensityToScore(segs, scoreIn, scoreOut, /*mapTo357=*/true)) {
        cerr << "[ERROR] 악보 처리 실패: " << scoreIn << " → " << scoreOut << "\n";
        return 2;
    }

    cout << "[완료] 세기 반영 CSV 저장: " << scoreOut << "\n";
    return 0;
}
