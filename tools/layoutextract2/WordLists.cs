using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace LayoutExtract2
{
   public class WordLists
   {
      // Used to generate easy to recognize names for the objects, keys and actions
      public WordLists( System.Random random )
      {
         Dictionary<string, int> transitiveVerbs = new Dictionary<string, int>();
         Dictionary<string, int> nouns = new Dictionary<string, int>();

         string assemblyName = System.Reflection.Assembly.GetExecutingAssembly().Location;
         string dictionaryName = System.IO.Path.GetDirectoryName(assemblyName) + "\\part-of-speech.txt";
         StreamReader sr = File.OpenText(dictionaryName);

         // The format of each entry is <word><tab><POS tag>[|<POS tag>]<unix newline>
         // Where the POS tag is one or more of the following:
         // N	Noun
         // h	Noun Phrase
         // V	Verb (usu participle)
         // t	Verb (transitive)
         // ...
         // Taken from the Part Of Speech database at http://wordlist.sourceforge.net/

         char[] wordTagSplitter = new char[] { '\t' };
         char[] wordSplitter = new char[] { ' ' };
         char[] tagSplitter = new char[] { '|' };

         while (true)
         {
            string line = sr.ReadLine();
            if (line == null)
            {
               break;
            }
            string[] parts = line.Split(wordTagSplitter, StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 2)
            {
               string phrase = parts[0];
               string posTag = parts[1];
               string[] posTags = posTag.Split(tagSplitter, StringSplitOptions.RemoveEmptyEntries);
               bool isTransitiveVerb = false;
               bool isNoun = false;
               foreach (string tag in posTags)
               {
                  // too many special cases without relying on V to indicate the participle
                  if (tag.Equals("tV"))// || (tag.Equals("t") && !word.EndsWith("ing") && !word.EndsWith("ed")))
                  {
                     isTransitiveVerb = true;
                  }
                  if (tag.Equals("h") || tag.Equals("N"))
                  {
                     isNoun = true;
                  }
               }
               if (isTransitiveVerb || isNoun)
               {
                  string[] words = phrase.Split(wordSplitter, StringSplitOptions.RemoveEmptyEntries);
                  if (words.Length == 1)
                  {
                     string word = words[0];
                     string wordLower = word.ToLower();
                     bool isAllLetters = true;
                     foreach (char c in wordLower.ToCharArray())
                     {
                        if (c < 'a' || c > 'z')
                        {
                           isAllLetters = false;
                           break;
                        }
                     }
                     // 7 gives the most verbs and almost the most nouns
                     if (word.Length == 7 && isAllLetters && word[1] >= 'a' && word[1] <= 'z')
                     {
                        word = System.Threading.Thread.CurrentThread.CurrentCulture.TextInfo.ToTitleCase(wordLower);
                        if (isTransitiveVerb)
                        {
                           // using dictionaries because there can be capitalized and uncapitalized versions of the same word
                           if (!transitiveVerbs.ContainsKey(word))
                           {
                              transitiveVerbs.Add(word, 1);
                           }
                        }
                        else
                        {
                           if (!nouns.ContainsKey(word))
                           {
                              nouns.Add(word, 1);
                           }
                        }
                     }
                  }
               }
            }
         }

         mTransitiveVerbs = new List<string>(transitiveVerbs.Keys);
         mNouns = new List<string>(nouns.Keys);

         Reset( random );
      }
      public void Reset( System.Random random )
      {
         mNextVerb = 0;
         mNextNoun = 0;
         mNextVerbRollover = 0;
         mNextNounRollover = 0;


         // now shuffle
         ShuffleList(mTransitiveVerbs, random);
         ShuffleList(mNouns, random);
      }
      public string GetRandomVerb()
      {
         if (mNextVerb == mTransitiveVerbs.Count)
         {
            mNextVerb = 0;
            mNextVerbRollover++;
         }
         return mTransitiveVerbs[mNextVerb++] + ((mNextVerbRollover > 0) ? mNextVerbRollover.ToString() : "");
      }
      public string GetRandomNoun()
      {
         if (mNextNoun == mNouns.Count)
         {
            mNextNoun = 0;
            mNextNounRollover++;
         }
         return mNouns[mNextNoun++] + ((mNextNounRollover > 0) ? mNextNounRollover.ToString() : "");
      }
      void ShuffleList(List<string> list, System.Random random )
      {
         // there are only 4000 or so words so this isn't too inefficient
         // at any rate, much faster than pulling from one list and adding to another
         for (int i = 0; i < 6*list.Count; ++i)
         {
            int ri = random.Next(list.Count);
            int rj = random.Next(list.Count);
            string sw = list[ri];
            list[ri] = list[rj];
            list[rj] = sw;
         }
      }
      List<string> mTransitiveVerbs;
      List<string> mNouns;
      int mNextVerb;
      int mNextNoun;
      int mNextVerbRollover;
      int mNextNounRollover;
   }
}
