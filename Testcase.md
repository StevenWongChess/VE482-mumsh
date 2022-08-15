##### MileStone1

###### 1, read/parse/execute loop & `exit`

```bash
# test 1 (print exit and exit)
exit
# test 2 (shoule behave same as exit)
exit randomthing
```

###### 2, & 3, execute command with/without args

```bash
# test 1
ls
# test 2
ls -a
# test 3 (will output error)
diff
```

###### 4, redirection (> >> and <)

```bash
# test 1 (> will override)
echo 123 > 1.txt
echo 321 > 1.txt
cat 1.txt
# test 2 (> will append)
echo 123 > 1.txt
echo 321 >> 1.txt
cat 1.txt
# test 3
echo 123 > 1.txt
cat < 1.txt
# test 4 (combine >/>> and <)
cat < 1.txt >> 2.txt
```

###### 5, bash style  (> >> and < can be anywhere)

```bash
# test 1 (multi space ignored)
echo 123 abc
echo 123  abc 
# test 2 (anywhere)
< 1.txt > 2.txt cat
< 1.txt cat > 2.txt
# test 3 (no need for space)
<1.txt >2.txt cat
<1.txt cat>2.txt
```



##### MileStone2

###### 6, pipe

```bash
# test 1
ls | grep mum
# test 2 (parallel, set time limit 1s)
sleep 0.6 | sleep 0.6
# test 3 (tricky)
echo exit | ./mumsh
# test 4 (work with bash)
cat < 1.txt | cat > 2.txt
# test 5 (cascade pipes)
cat < 1.txt | cat | cat | cat | cat > 1.txt
```

###### 7, `Ctrl-D`

```bash
# test 1
echo 12ctrl-d3 ## will output 123
# test 2
ctrl-d ## exit 
```

###### 8, Built-in `pwd`, `cd` 

```bash
# test 1
pwd
# test 2
pwd | cat > 1.txt
# test 3
cd .
cd ..
cd ~
cd -
cd /dir
cd dir
```

###### 9, `Ctrl-C`

























