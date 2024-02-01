pipeline {
    agent { label 'osx && arm64' }

    options { 
        buildDiscarder(logRotator(numToKeepStr: '135', daysToKeepStr: '21'))
        gitLabConnection('GitLabConnectionJenkins')
    }
    
    stages {
        stage('Checkout macOS'){
            steps {
                deleteDir()
                checkout([
                    $class: 'GitSCM', 
                    branches: [[name: "origin/${env.BRANCH_NAME}"]],
                    userRemoteConfigs: [[ url: "${env.GIT_URL_SDK}", credentialsId: "12492eb8-0278-4402-98f0-4412abfb65c1" ]],
                    extensions: [
                        [$class: "UserIdentity",name: "jenkins", email: "jenkins@jenkins"],
                        ]
                ])
                script{
                    mac_sources_workspace = WORKSPACE
                }
            }
        }
        stage('Build macOS'){
            options{
                timeout(time: 120, unit: 'MINUTES')
            }
            environment{
                PATH = "/usr/local/bin:${env.PATH}"
                VCPKGPATH = "${env.HOME}/jenkins/vcpkg"
                BUILD_DIR = "build_dir"
                BUILD_DIR_X64 = "build_dir_x64"
            }
            steps{
                dir(mac_sources_workspace){
                    //Build SDK for arm64
                    sh "echo Building SDK for arm64"
                    sh "rm -rf ${BUILD_DIR}; mkdir ${BUILD_DIR}"
                    sh "cmake -DENABLE_SDKLIB_WERROR=ON -DENABLE_CHAT=ON -DCMAKE_BUILD_TYPE=Debug -DVCPKG_ROOT=${VCPKGPATH} -DCMAKE_VERBOSE_MAKEFILE=ON \
                    -S ${mac_sources_workspace} -B ${mac_sources_workspace}/${BUILD_DIR}"
                    sh "cmake --build ${mac_sources_workspace}/${BUILD_DIR} -j1"

                    //Build SDK for x64
                    sh "echo \"Building SDK for x64 (crosscompiling)\""
                    sh "rm -rf ${BUILD_DIR_X64}; mkdir ${BUILD_DIR_X64}"
                    sh "cmake -DENABLE_SDKLIB_WERROR=ON -DENABLE_CHAT=ON -DCMAKE_BUILD_TYPE=Debug -DVCPKG_ROOT=${VCPKGPATH} -DCMAKE_VERBOSE_MAKEFILE=ON \
                    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
                    -S ${mac_sources_workspace} -B ${mac_sources_workspace}/${BUILD_DIR_X64}"
                    sh "cmake --build ${mac_sources_workspace}/${BUILD_DIR_X64} -j1"
                }
            }
        }
        stage('Run macOS tests'){
            options{
                timeout(time: 250, unit: 'MINUTES')
            }
            environment { 
                MEGA_PWD = credentials('MEGA_PWD_DEFAULT')
                MEGA_PWD_AUX = credentials('MEGA_PWD_DEFAULT')
                MEGA_PWD_AUX2 = credentials('MEGA_PWD_DEFAULT')
                MEGA_REAL_PWD=credentials('MEGA_REAL_PWD_TEST')
                BUILD_DIR = "build_dir"  
                }
            steps{
                lock(label: 'SDK_Concurrent_Test_Accounts', variable: 'ACCOUNTS_COMBINATION', quantity: 1, resource: null){
                    dir(mac_sources_workspace){
                        script{
                            env.MEGA_EMAIL = "${env.ACCOUNTS_COMBINATION}"
                            echo "${env.ACCOUNTS_COMBINATION}"
                        }
                        sh "echo Running tests"
                        sh """#!/bin/zsh
                        set -x
                        cd ${env.BUILD_DIR}

                        ./tests/unit/test_unit &
                        pid=\$!
                        wait \$pid || FAILED=1

                        if [ -z \"\$FAILED\" ]; then
                            if [ -z \"${TESTS_PARALLEL}\" ]; then
                                # Sequential run
                                ./tests/integration/test_integration --CI --USERAGENT:${env.USER_AGENT_TESTS} --APIURL:${APIURL_TO_TEST} &
                                pid=\$!
                                wait \$pid || FAILED=2
                            else
                                # Parallel run
                                ./tests/integration/test_integration --CI --USERAGENT:${env.USER_AGENT_TESTS} --APIURL:${APIURL_TO_TEST} ${TESTS_PARALLEL} 2>&1 | tee tests.stdout
                                [ \"\${pipestatus[1]}\" != \"0\" ] && FAILED=2
                            fi
                        fi

                        if [ -n \"\$FAILED\" ]; then
                            if [ \"\$FAILED\" -eq 1 ]; then
                                procFailed=\$pid
                            else # FAILED=2
                                if [ -z \"${TESTS_PARALLEL}\" ]; then
                                    # Sequential run
                                    procFailed=\$pid
                                else
                                    # Parallel run
                                    procFailed=`grep \"<< PROCESS\" tests.stdout | sed 's/.*PID:\\([0-9]*\\).*/\\1/' | tr '\\n' ' '`
                                fi
                            fi
                            if [ -n \"\$procFailed\" ]; then
                                sleep 10
                                for i in `echo \$procFailed`; do
                                    last_core=`grep \"test_.*\$i\" -rn \$HOME/Library/Logs/DiagnosticReports | cut -d':' -f1`
                                    if [ -n \"\$last_core\" ]; then
                                        cat \"\$last_core\"
                                        rm \"\$last_core\"
                                    fi
                                done
                            fi
                        fi

                        gzip -c test_integration.log > test_integration_${BUILD_ID}.log.gz || :
                        rm test_integration.log || :
                        if [ -n \"\$FAILED\" ]; then
                            exit \"\$FAILED\"
                        fi
                        """
                    }
                }
            }
        }
    }
}
