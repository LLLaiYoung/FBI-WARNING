#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "FWPluginPropertyCheck.hpp"

using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;

namespace FWPlugin {
    
    class FWMatchHandler: public MatchFinder::MatchCallback {
    private:
        CompilerInstance &CI;
        
        FixItHint replacementFixItHint(const NamedDecl *nameDecl, const string replaceStr, string name="") {
            if (name == "") {
                name = nameDecl->getNameAsString();
            }
            SourceLocation nameStart = nameDecl->getLocation();
            SourceLocation nameEnd = nameStart.getLocWithOffset(name.size() - 1);
            return FixItHint::CreateReplacement(SourceRange(nameStart, nameEnd), replaceStr);
        }
        
        FixItHint uppercaseFixItHint(const NamedDecl *nameDecl) {
            string tempName = nameDecl->getNameAsString();
            tempName[0] = fw_toUppercase(tempName[0]);
            return replacementFixItHint(nameDecl, tempName);
        }
        
        FixItHint lowercaseFixItHint(const NamedDecl *nameDecl, string name="") {
            if (name == "") {
                name = nameDecl->getNameAsString();
            }
            name[0] = fw_toLowercase(name[0]);
            return replacementFixItHint(nameDecl, name, name);
        }
        
        void handleInterfaceOrImplDecl(const NamedDecl *nameDecl) {
            const string name = nameDecl->getNameAsString();
            
            if (isLowercaseName(name)) {
                DiagnosticsEngine &diag = CI.getDiagnostics();
                diag.Report(nameDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 类名应该使用大写开头"));//.AddFixItHint(uppercaseFixItHint(interfaceDecl));
            }
        }
        
        void handlePropertyDecl(const ObjCPropertyDecl *propertyDecl) {
            ObjCPropertyDecl::PropertyAttributeKind attrKind = propertyDecl->getPropertyAttributes();
            const string typeStr = propertyDecl->getType().getAsString();
            
            if (propertyDecl->getTypeSourceInfo()) {
                DiagnosticsEngine &diag = CI.getDiagnostics();
                if (isShouldUseCopy(typeStr)) {
                    if (!(attrKind & ObjCPropertyDecl::OBJC_PR_copy)) {
                        diag.Report(propertyDecl->getBeginLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING %0 应该使用 copy 修饰")) << typeStr;
                    }
                } else if (isShouldUseWeak(typeStr) && !(attrKind & ObjCPropertyDecl::OBJC_PR_weak)) {
                    diag.Report(propertyDecl->getBeginLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING %0 应该使用 weak 修饰")) << typeStr;
                }
                
                if (!(attrKind & ObjCPropertyDecl::OBJC_PR_nonatomic)) {
                    diag.Report(propertyDecl->getBeginLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 请注意原子性")) << typeStr;
                }
                
                const string propertyName = propertyDecl->getNameAsString();
                if (isUppercaseName(propertyName)) {
                    diag.Report(propertyDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING Property name 应该使用小写开头"));//.AddFixItHint(lowercaseFixItHint(propertyDecl));
                }
                
                if (isUsedMemoryKeyword(propertyName)) {
                    diag.Report(propertyDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING Property name 不应该使用内存管理语义命名"));
                }
            }
        }
        
        void handleMethodDecl(const ObjCMethodDecl *methodDecl) {
            const string methodName = methodDecl->getSelector().getAsString();
            DiagnosticsEngine &diag = CI.getDiagnostics();
            
            if (!methodDecl->isPropertyAccessor()) {
                if (isMethodUppercaseName(methodName)) {
                    // lowercaseFixItHint 位置不对
                    diag.Report(methodDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 方法名应该使用小写开头"));//.AddFixItHint(lowercaseFixItHint(methodDecl, methodName));
                }
                
                if (isUsedMemoryKeyword(methodName)) {
                    diag.Report(methodDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 方法名不应该使用内存管理语义命名"));
                }
                
                for (ObjCMethodDecl::param_const_iterator it = methodDecl->param_begin(); it != methodDecl->param_end(); it++) {
                    const ParmVarDecl *parmVarDecl = *it;
                    string parmName = parmVarDecl->getNameAsString();
                    if (isUppercaseName(parmName)) {
                        diag.Report(parmVarDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 参数名应该使用小写开头"));//.AddFixItHint(lowercaseFixItHint(parmVarDecl, parmName));
                    }
                }
            }
            
            if (methodDecl->hasBody()) {
                Stmt *methodBody = methodDecl->getBody();
                string srcCode;
                srcCode.assign(CI.getSourceManager().getCharacterData(methodBody->getSourceRange().getBegin()),
                               methodBody->getSourceRange().getEnd().getRawEncoding() - methodBody->getSourceRange().getBegin().getRawEncoding() + 1);
                vector<string> lines = split(srcCode, '\n');
                if (lines.size() > 50) {
                    diag.Report(methodDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 超过 50 行限制"));
                }
            }
        }
        
        void handleEnumDecl(const EnumDecl *enumDecl) {
            LangOptions LangOpts;
            LangOpts.ObjC = true;
            PrintingPolicy Policy(LangOpts);
            
            string sExpr;
            raw_string_ostream rsoExpr(sExpr);
            enumDecl->print(rsoExpr,Policy);
            string oriExpr = rsoExpr.str();
            remove_blank(oriExpr);
            if(oriExpr.find("typedefenum")==0 || oriExpr.find("enum")==0){
                DiagnosticsEngine &diag = CI.getDiagnostics();
                diag.Report(enumDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING Use NS_ENUM/NS_OPTIONS instead of enum."));
            }
        }
        
        void handleIvarDecl(const ObjCIvarDecl *ivarDecl) {
            if (!ivarDecl->getSynthesize()) {
                DiagnosticsEngine &diag = CI.getDiagnostics();
                diag.Report(ivarDecl->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 应该使用 @Property 方式"));
            }
        }
        
        void handleCategory(const ObjCCategoryDecl *category) {
            const string interfaceName = category->getClassInterface()->getNameAsString();
            
            DiagnosticsEngine &diag = CI.getDiagnostics();
            if (isLowercaseName(interfaceName)) {
                diag.Report(category->getLocation(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 类名应该使用大写开头"));//.AddFixItHint(uppercaseFixItHint(interfaceDecl));
            }
            
            const string categoryName = category->getNameAsString();
            if (categoryName != "") {
                if (!has_Prefix(categoryName, "FW") || isLowercaseName(categoryName)) {
                    diag.Report(category->getCategoryNameLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 分类名应该使用“FW”大写开头"));
                }
                
                size_t length = categoryName.length();
                if ((length > 2 && isLowercase(categoryName[2])) || length < 2) {
                    diag.Report(category->getCategoryNameLoc(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "❌ ⚠️ FBI WARNING 分类名应该使用驼峰命名"));
                }
            }
        }
        
    public:
        FWMatchHandler(CompilerInstance &CI) :CI(CI) {}
        
        void run(const MatchFinder::MatchResult &Result) {
            const ObjCPropertyDecl *propertyDecl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("objcPropertyDecl");
            const ObjCProtocolDecl *protocolDecl = Result.Nodes.getNodeAs<ObjCProtocolDecl>("objcProtocolDecl");
            const ObjCInterfaceDecl *interfaceDecl = Result.Nodes.getNodeAs<ObjCInterfaceDecl>("objecInterfaceDecl");
            const ObjCImplementationDecl *implDecl = Result.Nodes.getNodeAs<ObjCImplementationDecl>("objcImplementationDecl");
            const ObjCCategoryDecl *categoryDecl = Result.Nodes.getNodeAs<ObjCCategoryDecl>("objcCategoryDecl");
            const ObjCMethodDecl *methodDecl = Result.Nodes.getNodeAs<ObjCMethodDecl>("objcMethodDecl");
            const EnumDecl *enumDecl = Result.Nodes.getNodeAs<EnumDecl>("enumDecl");
            const ObjCIvarDecl *ivarDecl = Result.Nodes.getNodeAs<ObjCIvarDecl>("objcIvarDecl");
            
            if (protocolDecl) {
                protocolDecl_puch_back(protocolDecl->getObjCRuntimeNameAsString());
            }
            
            if ((interfaceDecl && isUserSourceCode(CI.getSourceManager().getFilename(interfaceDecl->getSourceRange().getBegin()).str())) ||
                (implDecl && isUserSourceCode(CI.getSourceManager().getFilename(implDecl->getSourceRange().getBegin()).str()))) {
                const NamedDecl *nameDecl = interfaceDecl;
                if (!nameDecl) nameDecl = implDecl;
                handleInterfaceOrImplDecl(nameDecl);
            }
            
            if (categoryDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(categoryDecl->getSourceRange().getBegin()).str())) {
                handleCategory(categoryDecl);
            }
            
            if (propertyDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(propertyDecl->getSourceRange().getBegin()).str())) {
                handlePropertyDecl(propertyDecl);
            }
            
            if (methodDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(methodDecl->getSourceRange().getBegin()).str())) {
                handleMethodDecl(methodDecl);
            }
            
            if (enumDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(enumDecl->getSourceRange().getBegin()).str())) {
                handleEnumDecl(enumDecl);
            }
            
            if (ivarDecl &&
                isUserSourceCode(CI.getSourceManager().getFilename(ivarDecl->getSourceRange().getBegin()).str())) {
                handleIvarDecl(ivarDecl);
            }
        }
    };
    
    class FWASTConsumer: public ASTConsumer {
    private:
        MatchFinder matcher;
        FWMatchHandler handler;
    public:
        FWASTConsumer(CompilerInstance &CI) :handler(CI) {
            matcher.addMatcher(objcPropertyDecl().bind("objcPropertyDecl"), &handler);
            matcher.addMatcher(objcProtocolDecl().bind("objcProtocolDecl"), &handler);
            matcher.addMatcher(objcInterfaceDecl().bind("objecInterfaceDecl"), &handler);
            matcher.addMatcher(objcImplementationDecl().bind("objcImplementationDecl"), &handler);
            matcher.addMatcher(objcCategoryDecl().bind("objcCategoryDecl"), &handler);
            matcher.addMatcher(objcMethodDecl().bind("objcMethodDecl"), &handler);
            matcher.addMatcher(enumDecl().bind("enumDecl"), &handler);
            matcher.addMatcher(objcIvarDecl().bind("objcIvarDecl"), &handler);
        }
        
        void HandleTranslationUnit(ASTContext &context) {
            matcher.matchAST(context);
        }
    };
    
    class FWASTAction: public PluginASTAction {
    public:
        unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef iFile) {
            return unique_ptr<FWASTConsumer> (new FWASTConsumer(CI));
        }
        
        bool ParseArgs(const CompilerInstance &ci, const std::vector<std::string> &args) {
            return true;
        }
    };
}

static FrontendPluginRegistry::Add<FWPlugin::FWASTAction> X("FWPlugin", "A custom static code analysis of Objc");
