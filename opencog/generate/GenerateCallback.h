/*
 * opencog/generate/GenerateCallback.h
 *
 * Copyright (C) 2020 Linas Vepstas <linasvepstas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _OPENCOG_GENERATE_CALLBACK_H
#define _OPENCOG_GENERATE_CALLBACK_H

#include <opencog/atomspace/AtomSpace.h>
#include <opencog/generate/Frame.h>

namespace opencog
{
/** \addtogroup grp_generate
 *  @{
 */

/// Aggregation selection callbacks. As an assembly is being created,
/// that assembly will have unconnected, open connectors on it.
/// Aggregation proceeds by attaching sections ("puzzle pieces") to
/// each open connector, until there are none left. But what to connect
/// to what? The master aggregation algorithm defers that choice to this
/// callback API.  Different algos can offer up different connections to
/// try out. The master aggregation algo manages the overall process of
/// connecting things up; the callbacks suggest what to connect next.
///
/// The master aggregation algo is stack-based, and potentially
/// combinatoric-explosive, as each the current set of open connectors
/// depends on the history of what was previously attached. Because
/// the algo is breadt-first (see other descriptions) two stacks are
/// maintained: one for each "row" (odometer) and one for each
/// odometer-wheel.
///
class GenerateCallback
{
public:
	GenerateCallback(AtomSpace* as) {}
	virtual ~GenerateCallback() {}

	/// Given a connector, return a set of matching connectors
	/// that this particular connector could connect to. This
	/// set may be empty, or may contain more than one match.
	virtual HandleSeq joints(const Handle&) = 0;

	/// Given an existing connected section `fm_sect` and a connector
	/// `fm_con` on that section, as well as a mating `to_con`, return
	/// a section that could be attached. This allows the callback to
	/// chose sections in such a way that the highest-priority or most
	/// preferable sections are mated first.  Return null handle to
	/// discontinue mating.
	///
	/// The `fm_con` is the connector located at `offset` in the
	/// `fm_sect`.
	///
	/// This should be implemented so that it behaves like a 'future'
	/// or a 'promise', so that, when called, it returns the next
	/// section from a (virtual) list of eligible sections.
	virtual Handle select(const Frame&,
	                      const Handle& fm_sect, size_t offset,
	                      const Handle& to_con) = 0;

	/// Create a link from connector `fm_con` to connector `to_con`,
	/// which will connect `fm_pnt` to `to_pnt`.
	virtual Handle make_link(const Handle& fm_con, const Handle& to_con,
	                         const Handle& fm_pnt, const Handle& to_pnt) = 0;

	/// Return a link, but only if it already exists; do not make a
	/// a new link if it doesn't. Otherwise, much like `make_link()`:
	/// Return a link from connector `fm_con` to connector `to_con`,
	/// which will connect `fm_pnt` to `to_pnt`.
	virtual Handle have_link(const Handle& fm_con, const Handle& to_con,
	                         const Handle& fm_pnt, const Handle& to_pnt) = 0;

	virtual void push_frame(const Frame&) {}
	virtual void pop_frame(const Frame&) {}

	virtual void push_odometer(const Odometer&) {}
	virtual void pop_odometer(const Odometer&) {}

	/// Called before taking a step of the odometer.
	/// Return `true` to take the step, else false.
	/// Returning false will abort the current odometer.
	/// Traversal will resume at an earlier level.
	///
	/// The default below allows infinite recursion.
	virtual bool step(const Frame&) { return true; }

	/// Called when a solution is found. A solution is a linkage,
	/// with no open connectors.
	virtual void solution(const Frame&) = 0;

	// ---------------------------------------------------------------
	/// Generic Parameters
	/// These are parameters that all callback systems might reasonably
	/// want to consult, when determining behavior.
	// (Should these be moved to thier own class???)

	/// Maximum number of solutions to accept. Search is halted after
	/// this number is reached.
	size_t max_solutions = -1;

	/// Allow connectors on an open section to connect back onto
	/// themselves (if the other mating rules allow the two connectors
	/// to connect).
	bool allow_self_connections = false;

	/// The maximum number of links allowed between a pair of sections.
	/// By default, it is one, as "most" "typical" graphs make sense
	/// when only one edge connects a pair of vertexes.
	size_t max_pair_links = 1;

	/// Maximum size of the generated network. Exploration of networks
	/// larger than this will not be attempted.
	size_t max_network_size = -1;

	/// Maximum depth to explore from the starting point. This is
	/// counted in terms of the maximum depth of the stack of
	/// odometers. This is maximum diameter of the network, as measured
	/// from the starting point.
	size_t max_depth = -1;
};


/** @}*/
}  // namespace opencog

#endif // _OPENCOG_GENERATE_CALLBACK_H
