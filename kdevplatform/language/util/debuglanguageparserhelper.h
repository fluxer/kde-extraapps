/* This file is part of KDevelop
    Copyright 2010 Milian Wolff

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H
#define KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H

#include <QTextStream>

#include <cstdlib>
#include <cstdio>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KApplication>

#include <tests/autotestshell.h>
#include <language/duchain/duchain.h>
#include <language/duchain/problem.h>
#include <language/codegen/coderepresentation.h>
#include <tests/testcore.h>

namespace KDevelopUtils {


QTextStream qout(stdout);
QTextStream qerr(stderr);
QTextStream qin(stdin);

/**
 * This class is a pure helper to use for binaries that you can
 * run on short snippets of test code or whole files and let
 * it print the generated tokens or AST.
 *
 * It should work fine for any KDevelop-PG-Qt based parser.
 *
 *
 * @param SessionT the parse session for your language.
 * @param TokenStreamT the token stream for your language, based on KDevPG::TokenStreamBase.
 * @param TokenT the token class for your language, based on KDevPG::Token.
 * @param LexerT the Lexer for your language.
 * @param StartAstT the AST node that is returned from @c SessionT::parse().
 * @param DebugVisitorT the debug visitor for your language.
 * @param TokenToTextT function pointer to the function that returns a string representation for an integral token.
 */
typedef QString (*TokenTextFunc)(int);
template<class SessionT, class TokenStreamT, class TokenT, class LexerT,
         class StartAstT, class DebugVisitorT, TokenTextFunc TokenTextT>
class DebugLanguageParserHelper {
public:
    DebugLanguageParserHelper(const bool printAst, const bool printTokens)
        : m_printAst(printAst), m_printTokens(printTokens)
    {
        m_session.setDebug(printAst);
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
        if (!m_session.readFile(fileName, "utf-8")) {
            qerr << "Can't open file " << fileName << endl;
            std::exit(255);
        } else {
            qout << "Parsing file " << fileName << endl;
        }
        runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
        m_session.setContents(code);

        qout << "Parsing input" << endl;
        runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
        if (m_printTokens) {
            TokenStreamT tokenStream;
            LexerT lexer(&tokenStream, m_session.contents());
            int token;
            while ((token = lexer.nextTokenKind())) {
                TokenT &t = tokenStream.push();
                t.begin = lexer.tokenBegin();
                t.end = lexer.tokenEnd();
                t.kind = token;
                printToken(token, lexer);
            }
            printToken(token, lexer);
            if ( tokenStream.size() > 0 ) {
                qint64 line;
                qint64 column;
                tokenStream.endPosition(tokenStream.size() - 1, &line, &column);
                qDebug() << "last token endPosition: line" << line << "column" << column;
            } else {
                qDebug() << "empty token stream";
            }
        }

        StartAstT* ast = 0;
        if (!m_session.parse(&ast)) {
            qerr << "no AST tree could be generated" << endl;
        } else {
            qout << "AST tree successfully generated" << endl;
            if (m_printAst) {
                DebugVisitorT debugVisitor(m_session.tokenStream(), m_session.contents());
                debugVisitor.visitStart(ast);
            }
        }
        if (!m_session.problems().isEmpty()) {
            qout << endl << "problems encountered during parsing:" << endl;
            foreach(KDevelop::ProblemPointer p, m_session.problems()) {
                qout << p->description() << endl;
            }
        } else {
            qout << "no problems encountered during parsing" << endl;
        }

        if (!ast) {
            exit(255);
        }
    }

    void printToken(int token, const LexerT& lexer) const
    {
        int begin = lexer.tokenBegin();
        int end = lexer.tokenEnd();
        qout << m_session.contents().mid(begin, end - begin + 1).replace('\n', "\\n")
             << ' ' << TokenTextT(token) << endl;
    }

    SessionT m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

/// call this after setting up @p aboutData in your @c main() function.
template<class ParserT>
int initAndRunParser(const KAboutData& aboutData, int argc, char* argv[])
{
    qout.setCodec("UTF-8");
    qerr.setCodec("UTF-8");
    qin.setCodec("UTF-8");

    KCmdLineArgs::init( argc, argv, &aboutData, KCmdLineArgs::CmdLineArgNone );
    KCmdLineOptions options;
    options.add("a").add("print-ast", ki18n("print generated AST tree"));
    options.add("t").add("print-tokens", ki18n("print generated token stream"));
    options.add("c").add("code <code>", ki18n("code to parse"));
    options.add("+files", ki18n("files or - to read from STDIN, the latter is the default if nothing is provided"));
    KCmdLineArgs::addCmdLineOptions( options );

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication app(false);

    QStringList files;
    for ( int i = 0; i < args->count(); ++i ) {
        files << args->arg(i);
    }

    bool printAst = args->isSet("print-ast");
    bool printTokens = args->isSet("print-tokens");

    KDevelop::AutoTestShell::init();
    KDevelop::TestCore::initialize(KDevelop::Core::NoUi);

    KDevelop::DUChain::self()->disablePersistentStorage();
    KDevelop::CodeRepresentation::setDiskChangesForbidden(true);

    ParserT parser(printAst, printTokens);

    if ( args->isSet("code") ) {
        parser.parseCode( args->getOption("code") );
    } else if ( files.isEmpty() ) {
        files << "-";
    }

    foreach(const QString &fileName, files) {
        if ( fileName == "-" ) {
            if ( isatty(STDIN_FILENO) ) {
                qerr << "no STDIN given" << endl;
                return 255;
            }
            parser.parseCode( qin.readAll().toUtf8() );
        } else {
            parser.parseFile(fileName);
        }

    }

    KDevelop::TestCore::shutdown();

    return 0;
}
}

#endif // KDEVPLATFORM_DEBUGLANGUAGEPARSERHELPER_H
