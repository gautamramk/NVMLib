#include <iostream>
#include <unordered_set>
// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "attribs.h"
#include "stringpool.h"
#include "pretty-print.h"
#include <string.h>

int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = { "1.0", "Memory Reference Tracker" };
tree rfndecl;
tree wfndecl;

namespace
{
    const pass_data mem_check_data = 
    {
        GIMPLE_PASS,
        "mem_check",        /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct mem_check_pass : gimple_opt_pass
    {
        mem_check_pass(gcc::context *ctx)
            : gimple_opt_pass(mem_check_data, ctx)
        {

        }

        virtual unsigned int execute(function* fun) override
        {
            // fun is the current function being called

            gimple_seq gimple_body = fun->gimple_body;

            //std::cerr << "FUNCTION '" << function_name(fun)
            //    << "' at " << LOCATION_FILE(fun->function_start_locus) << ":" << LOCATION_LINE(fun->function_start_locus) << "\n";
            //std::cerr << "*******************\n";

            // Dump its body
            //print_gimple_seq(stderr, gimple_body, 0, 0);
            gimple_stmt_iterator it = gsi_start_1(&gimple_body);
            for(; !gsi_end_p(it); gsi_next(&it)) {
                if (gsi_stmt(it)->code == GIMPLE_CALL) {
                    tree ret_tree = gimple_call_lhs(gsi_stmt(it));
                    tree fn_tree = gimple_call_fndecl(gsi_stmt(it));
                    gimple* last_stmt = gsi_stmt(it);
                    if (!strcmp(IDENTIFIER_POINTER(DECL_NAME(fn_tree)), "get_memobj_direct")) {
                        //printf("GV_________________\n");

                        tree fntype = build_function_type_list(void_type_node, NULL_TREE);
                        tree ldecl = build_fn_decl ("lock_om", fntype);
                        gimple *lcall = gimple_build_call (ldecl, 0);
                        gsi_prev(&it);
                        gsi_insert_before(&it, lcall, GSI_SAME_STMT);
                        gsi_next(&it);

                        std::unordered_set<tree> vars;
                        vars.insert(ret_tree);
                        gimple_stmt_iterator end_it = it;
                        for(; !gsi_end_p(end_it); gsi_next(&end_it)) {
                            debug_gimple_stmt(gsi_stmt(end_it));
                            if (gsi_stmt(end_it)->code == GIMPLE_ASSIGN) {                               
                               if (TREE_CODE(gimple_assign_lhs(gsi_stmt(end_it))) == MEM_REF) {
                                   tree reffed = gimple_assign_lhs(gsi_stmt(end_it));
                                   tree reffed_id = TREE_OPERAND(reffed, 0);
                                   if (vars.find(reffed_id) != vars.end()) {
                                       //printf("unlock here\n");
                                       last_stmt = gsi_stmt(end_it);
                                       break;
                                   }
                               } else if (TREE_CODE(gimple_assign_lhs(gsi_stmt(end_it))) == SSA_NAME ||
                                          TREE_CODE(gimple_assign_lhs(gsi_stmt(end_it))) == VAR_DECL) {
                                    tree rhs1, rhs2, rhs3;
                                    rhs1 = gimple_assign_rhs1(gsi_stmt(end_it));
                                    rhs2 = gimple_assign_rhs2(gsi_stmt(end_it));
                                    rhs3 = gimple_assign_rhs3(gsi_stmt(end_it));
                                    if (vars.find(rhs1) != vars.end() || vars.find(rhs2) != vars.end() ||
                                        vars.find(rhs3) != vars.end()) {
                                            vars.insert(gimple_assign_lhs(gsi_stmt(end_it)));
                                    }
                                }
                                if (gimple_assign_rhs_code(gsi_stmt(end_it)) == MEM_REF) {
                                    tree reffed = TREE_OPERAND(gimple_assign_rhs1(gsi_stmt(end_it)), 0);
                                    if (vars.find(reffed) != vars.end()) {
                                        last_stmt = gsi_stmt(end_it);
                                        //printf("Could end here.\n");
                                    }
                                }
                            }
                        }
                        end_it.ptr = last_stmt;
                        tree udecl = build_fn_decl ("unlock_om", fntype);
                        gimple *ucall = gimple_build_call (udecl, 0);
                        gsi_insert_after(&end_it, ucall, GSI_NEW_STMT);
                    }
                }
            }
            printf("______________________________\n");
            gimple_stmt_iterator it1 = gsi_start_1(&gimple_body);
            for(; !gsi_end_p(it1); gsi_next(&it1)) {
                debug_gimple_stmt(gsi_stmt(it1));
            }

            it = gsi_start_1(&gimple_body);
            for(; !gsi_end_p(it); gsi_next(&it)) {
                struct walk_stmt_info walk_stmt_info;
                memset(&walk_stmt_info, 0, sizeof(walk_stmt_info));
                int mem_ref = 0;
                walk_stmt_info.info = &mem_ref;
                //print_gimple_expr(stderr, gsi_stmt(it), 100, 100);
                //std::cerr << "\n";
                //walk_gimple_op(gsi_stmt(it), callback_op, &walk_stmt_info);
                //if (mem_ref == 1){
                if (gsi_stmt(it)->code == GIMPLE_ASSIGN){
                    if (TREE_CODE(gimple_assign_lhs(gsi_stmt(it))) == MEM_REF) {
                        tree lt = TREE_OPERAND(gimple_assign_lhs(gsi_stmt(it)), 0);
                        tree lty = TREE_TYPE(lt);
                        tree ltyty = TREE_TYPE(lty);
                        tree ltsiz = TYPE_SIZE(ltyty);
                        tree fntype = build_function_type_list(void_type_node, ptr_type_node, integer_type_node, NULL_TREE);
                        wfndecl = build_fn_decl ("log_write", fntype);
                        gimple *call = gimple_build_call (wfndecl, 2, lt, ltsiz);
 
                        //dump_function_header(stderr, rfndecl,0);
                        gimple_set_location(call, gimple_location(gsi_stmt(it)));
                        gsi_insert_after(&it, call, GSI_NEW_STMT);

                    }
                    if (gimple_assign_rhs_code(gsi_stmt(it)) == MEM_REF) {
                        tree lt = TREE_OPERAND(gimple_assign_rhs1(gsi_stmt(it)),0);
                        tree lty = TREE_TYPE(lt);
                        tree ltyty = TREE_TYPE(lty);
                        tree ltsiz = TYPE_SIZE(ltyty);

                        tree fntype = build_function_type_list(void_type_node, ptr_type_node, integer_type_node, NULL_TREE);
                        rfndecl = build_fn_decl ("log_read", fntype);
                        gimple *call = gimple_build_call (rfndecl, 2, lt, ltsiz);
 
                        //dump_function_header(stderr, rfndecl,0);
                        gimple_set_location(call, gimple_location(gsi_stmt(it)));
                        gsi_insert_after(&it, call, GSI_NEW_STMT);
                    }
                }
            }

            //std::cerr << "*******************\n\n";

            return 0;
        }

        static tree callback_op (tree *t, int *, void *data)
        {
            enum tree_code code = TREE_CODE(*t);

            if (code == MEM_REF) {
                *((int*)((struct walk_stmt_info*)data)->info) = 1;
            }
            return NULL;
        }

        virtual mem_check_pass* clone() override
        {
            // We do not clone ourselves
            return this;
        }
    };
}


int plugin_init (struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version)
{
	// We check the current gcc loading this plugin against the gcc we used to
	// created this plugin
	if (!plugin_default_version_check (version, &gcc_version))
    {
        std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
    }

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &my_gcc_plugin_info);

    // Register the phase right after omplower
    struct register_pass_info pass_info;

    // Note that after the cfg is built, fun->gimple_body is not accessible
    // anymore so we run this pass just before it
    pass_info.pass = new mem_check_pass(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_BEFORE;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

    return 0;
}