/*******************************************************************************
 * Copyright (c) 2019, 2019 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at https://www.eclipse.org/legal/epl-2.0/
 * or the Apache License, Version 2.0 which accompanies this distribution and
 * is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following
 * Secondary Licenses when the conditions for such availability set
 * forth in the Eclipse Public License, v. 2.0 are satisfied: GNU
 * General Public License, version 2 with the GNU Classpath
 * Exception [1] and GNU General Public License, version 2 with the
 * OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0 OR GPL-2.0 WITH Classpath-exception-2.0 OR LicenseRef-GPL-2.0 WITH Assembly-exception
 *******************************************************************************/

#if !defined(REALTIMEGCDELEGATE_HPP_)
#define REALTIMEGCDELEGATE_HPP_

#include "j9.h"
#include "j9cfg.h"

#if defined(J9VM_GC_REALTIME)

#include "omr.h"
#include "BaseNonVirtual.hpp"
#include "EnvironmentRealtime.hpp"
#include "GCExtensions.hpp"

class MM_RealtimeGCDelegate : public MM_BaseNonVirtual
{
private:
	MM_GCExtensions *_extensions;
	MM_RealtimeGC *_realtimeGC;
	J9JavaVM *_javaVM;

public:
#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
 	bool _unmarkedImpliesClasses; /**< if true the mark bit can be used to check is class alive or not */
#endif /* defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING) */
	
#if defined(J9VM_GC_FINALIZATION)	
	bool _finalizationRequired;
#endif /* J9VM_GC_FINALIZATION */
#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	bool _dynamicClassUnloadingEnabled;
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

	MM_RealtimeGCDelegate(MM_EnvironmentBase *env) :
		_extensions(MM_GCExtensions::getExtensions(env)),
		_realtimeGC(NULL),
		_javaVM((J9JavaVM*)env->getOmrVM()->_language_vm) {}

	bool initialize(MM_EnvironmentBase *env);
	void tearDown(MM_EnvironmentBase *env);

	bool allocateAndInitializeReferenceObjectLists(MM_EnvironmentBase *env);
	bool allocateAndInitializeUnfinalizedObjectLists(MM_EnvironmentBase *env);
	bool allocateAndInitializeOwnableSynchronizerObjectLists(MM_EnvironmentBase *env);

#if defined(J9VM_GC_FINALIZATION)	
	bool isFinalizationRequired() { return _finalizationRequired; }
#endif /* J9VM_GC_FINALIZATION */

	void masterSetupForGC(MM_EnvironmentBase *env);
	void masterCleanupAfterGC(MM_EnvironmentBase *env);
	void incrementalCollectStart(MM_EnvironmentRealtime *env);
	void incrementalCollect(MM_EnvironmentRealtime *env);
	void doAuxilaryGCWork(MM_EnvironmentBase *env);
	void clearGCStats(MM_EnvironmentBase *env);
	void reportSyncGCEnd(MM_EnvironmentBase *env);

#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	MMINLINE bool isDynamicClassUnloadingEnabled() { return _dynamicClassUnloadingEnabled; };
	void unloadDeadClassLoaders(MM_EnvironmentBase *env);

	void reportClassUnloadingStart(MM_EnvironmentBase *env);
	void reportClassUnloadingEnd(MM_EnvironmentBase *env);
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

#if defined(J9VM_GC_DYNAMIC_CLASS_UNLOADING)
	/**
	 * Perform initial cleanup for classloader unloading.  The current thread has exclusive access.
	 * The J9AccClassDying bit is set and J9HOOK_VM_CLASS_UNLOAD is triggered for each class that will be unloaded.
	 * The J9_GC_CLASS_LOADER_DEAD bit is set for each class loader that will be unloaded.
	 * J9HOOK_VM_CLASSES_UNLOAD is triggered if any classes will be unloaded.
	 * 
	 * @param env[in] the master GC thread
	 * @param classUnloadCountResult[out] returns the number of classes about to be unloaded
	 * @param anonymousClassUnloadCount[out] returns the number of anonymous classes about to be unloaded
	 * @param classLoaderUnloadCountResult[out] returns the number of class loaders about to be unloaded
	 * @param classLoaderUnloadListResult[out] returns a linked list of class loaders about to be unloaded
	 */
	void processDyingClasses(MM_EnvironmentRealtime *env, UDATA* classUnloadCountResult, UDATA* anonymousClassUnloadCount, UDATA* classLoaderUnloadCountResult, J9ClassLoader** classLoaderUnloadListResult);
	void processUnlinkedClassLoaders(MM_EnvironmentBase *env, J9ClassLoader *deadClassLoaders);
	void updateClassUnloadStats(MM_EnvironmentBase *env, UDATA classUnloadCount, UDATA anonymousClassUnloadCount, UDATA classLoaderUnloadCount);

	/**
	 * Scan classloader for dying classes and add them to the list
	 * @param env[in] the current thread
	 * @param classLoader[in] the list of class loaders to clean up
	 * @param setAll[in] bool if true if all classes must be set dying, if false unmarked classes only
	 * @param classUnloadListStart[in] root of list dying classes should be added to
	 * @param classUnloadCountOut[out] number of classes dying added to the list
	 * @return new root to list of dying classes
	 */
	J9Class *addDyingClassesToList(MM_EnvironmentRealtime *env, J9ClassLoader * classLoader, bool setAll, J9Class *classUnloadListStart, UDATA *classUnloadCountResult);
#endif /* J9VM_GC_DYNAMIC_CLASS_UNLOADING */

	void yieldFromClassUnloading(MM_EnvironmentRealtime *env);
	void lockClassUnloadMonitor(MM_EnvironmentRealtime *env);
	void unlockClassUnloadMonitor(MM_EnvironmentRealtime *env);

	UDATA getUnfinalizedObjectListCount(MM_EnvironmentBase *env) { return _extensions->gcThreadCount; }
	UDATA getOwnableSynchronizerObjectListCount(MM_EnvironmentBase *env) { return _extensions->gcThreadCount; }
	UDATA getReferenceObjectListCount(MM_EnvironmentBase *env) { return _extensions->gcThreadCount; }

	/*
	 * Friends
	 */
	friend class MM_RealtimeGC;
};

#endif /* defined(J9VM_GC_REALTIME) */

#endif /* defined(REALTIMEGCDELEGATE_HPP_) */	

