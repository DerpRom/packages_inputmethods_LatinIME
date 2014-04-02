/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "suggest/core/result/suggestion_results.h"

namespace latinime {

void SuggestionResults::outputSuggestions(JNIEnv *env, jintArray outSuggestionCount,
        jintArray outputCodePointsArray, jintArray outScoresArray, jintArray outSpaceIndicesArray,
        jintArray outTypesArray, jintArray outAutoCommitFirstWordConfidenceArray,
        jfloatArray outLanguageWeight) {
    int outputIndex = 0;
    while (!mSuggestedWords.empty()) {
        const SuggestedWord &suggestedWord = mSuggestedWords.top();
        suggestedWord.getCodePointCount();
        const int start = outputIndex * MAX_WORD_LENGTH;
        env->SetIntArrayRegion(outputCodePointsArray, start, suggestedWord.getCodePointCount(),
                suggestedWord.getCodePoint());
        if (suggestedWord.getCodePointCount() < MAX_WORD_LENGTH) {
            const int terminal = 0;
            env->SetIntArrayRegion(outputCodePointsArray, start + suggestedWord.getCodePointCount(),
                    1 /* len */, &terminal);
        }
        const int score = suggestedWord.getScore();
        env->SetIntArrayRegion(outScoresArray, outputIndex, 1 /* len */, &score);
        const int indexToPartialCommit = suggestedWord.getIndexToPartialCommit();
        env->SetIntArrayRegion(outSpaceIndicesArray, outputIndex, 1 /* len */,
                &indexToPartialCommit);
        const int type = suggestedWord.getType();
        env->SetIntArrayRegion(outTypesArray, outputIndex, 1 /* len */, &type);
        if (mSuggestedWords.size() == 1) {
            const int autoCommitFirstWordConfidence =
                    suggestedWord.getAutoCommitFirstWordConfidence();
            env->SetIntArrayRegion(outAutoCommitFirstWordConfidenceArray, 0 /* start */,
                    1 /* len */, &autoCommitFirstWordConfidence);
        }
        ++outputIndex;
        mSuggestedWords.pop();
    }
    env->SetIntArrayRegion(outSuggestionCount, 0 /* start */, 1 /* len */, &outputIndex);
    env->SetFloatArrayRegion(outLanguageWeight, 0 /* start */, 1 /* len */, &mLanguageWeight);
}

void SuggestionResults::addPrediction(const int *const codePoints, const int codePointCount,
        const int probability) {
    if (probability == NOT_A_PROBABILITY) {
        // Invalid word.
        return;
    }
    addSuggestion(codePoints, codePointCount, probability, Dictionary::KIND_PREDICTION,
            NOT_AN_INDEX, NOT_A_FIRST_WORD_CONFIDENCE);
}

void SuggestionResults::addSuggestion(const int *const codePoints, const int codePointCount,
        const int score, const int type, const int indexToPartialCommit,
        const int autocimmitFirstWordConfindence) {
    if (codePointCount <= 0 || codePointCount > MAX_WORD_LENGTH) {
        // Invalid word.
        AKLOGE("Invalid word is added to the suggestion results. codePointCount: %d",
                codePointCount);
        return;
    }
    if (getSuggestionCount() >= mMaxSuggestionCount) {
        const SuggestedWord &mWorstSuggestion = mSuggestedWords.top();
        if (score > mWorstSuggestion.getScore() || (score == mWorstSuggestion.getScore()
                && codePointCount < mWorstSuggestion.getCodePointCount())) {
            mSuggestedWords.pop();
        } else {
            return;
        }
    }
    mSuggestedWords.push(SuggestedWord(codePoints, codePointCount, score, type,
            indexToPartialCommit, autocimmitFirstWordConfindence));
}

void SuggestionResults::getSortedScores(int *const outScores) const {
    auto copyOfSuggestedWords = mSuggestedWords;
    while (!copyOfSuggestedWords.empty()) {
        const SuggestedWord &suggestedWord = copyOfSuggestedWords.top();
        outScores[copyOfSuggestedWords.size() - 1] = suggestedWord.getScore();
        copyOfSuggestedWords.pop();
    }
}

void SuggestionResults::dumpSuggestions() const {
    AKLOGE("language weight: %f", mLanguageWeight);
    std::vector<SuggestedWord> suggestedWords;
    auto copyOfSuggestedWords = mSuggestedWords;
    while (!copyOfSuggestedWords.empty()) {
        suggestedWords.push_back(copyOfSuggestedWords.top());
        copyOfSuggestedWords.pop();
    }
    int index = 0;
    for (auto it = suggestedWords.rbegin(); it != suggestedWords.rend(); ++it) {
        DUMP_SUGGESTION(it->getCodePoint(), it->getCodePointCount(), index, it->getScore());
        index++;
    }
}

} // namespace latinime
