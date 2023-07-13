> import Numeric (showHex)

> note :: Int -> Double
> note n = 3579545 / (32 * (440 * (2**(1/12))**(fromIntegral n)))

> notes :: [Int]
> notes = take 72 . map (round . note) $ [-21..]

 > high :: String
 > high = unlines ("static unsigned char const note_high {"
 >                 : map ('\t' :) (high' notes) ++ ["}"])
 > high' :: [Int] -> [String]
 > high' 

> hexify :: Int -> String
> hexify n = let x = showHex n ""
>            in replicate (2 - length x) '0' ++ x

> high :: Int -> String
> high n = "0x" ++ hexify (n `div` 16) ++ "U,"

> low :: Int -> String
> low n = "0x" ++ hexify (128 + (n `mod` 16)) ++ "U,"

> splitEvery :: Int -> [a] -> [[a]]
> splitEvery _ [] = []
> splitEvery n xs = take n xs : splitEvery n (drop n xs)

> highs = unlines ("static unsigned char const note_high[] = {"
>                  : map ('\t' :) block
>                 ++ ["};"])
>     where block = map unwords . splitEvery 6 $ map high notes
> lows = unlines ("static unsigned char const note_low[] = {"
>                 : map ('\t' :) block
>                ++ ["};"])
>     where block = map unwords . splitEvery 6 $ map low notes
> main = putStr lows >> putStr highs
