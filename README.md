## monitoring cross-site request forgery attacks with apache's modsecurity

<br>

### tl; dr

* a tool to monitor web server's log files against CSRF attacks, including a modification for modsecurity on apache 
* it is a nice example of a parser in ruby and ragel, together with some examples in C++ and python

<br>

---

### introduction

* designed to detect requests in a web server that may result in CSRF, by parsing a given log file derived from a web server with DIVA and verifying whether (and how) the requests in this file had the state of the system changed (named potential unsafe requests)

<br>

---

### parsing strategies


* parsing by regular expressions (REGEXP) (at `/lib/log_parser/strategies/sql/regexp.rb`)
* parsing into an abstract syntax tree (AST) (at `/lib/log_parser/strategies/sql/ragel.rl`)

<br>

---

### ragel state machine

* one can generate `.rb` files with:

```
ragel -R ragel.rl
```


* ragel is a state machine compiler and parse generator.
* it combines lex and yacc into one and build a full state-machine for the input stream, i.e., one state-machine for the parser and lexer.
* the machine of states parses the SQL request. in an initial state, it receives a string.
* if in the of the string, the machine is in a final state, the SQL is valid. the AST is a way the machine uses to save the data.
* the machine can get four initial paths: UPDATE, DELETE, SELECT, INSERT. it saves into the AST when the parse is executed.

```
ragel -R ragel.rl | rlgen-dot > ragel.dot
```

<br>

---

### whitelisting

* every time one runs MonCSRF, every the potential unsafe requests will be compared to a whitelist.
* in the case that program finds a previous similar whitelisted request (i.e., with same syntactic structure), the new request is automatically marked as safe. If the new request is not in the whitelist, the program will generate an alert and ask about its safety.
* the whitelist file can be inspected at `/white_list`.

<br>

---

### testings (for developers)

```
gem install bundler
bundle
spec spec
```

<br>

----

### running

* name the log file:

```
bin/parser.rb PATH_TO_THE_LOG
````

and run:

```
./run.sh
```

* run benchmark tests with:

```
./benchmark.sh
```
