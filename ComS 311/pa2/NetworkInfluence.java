// LEAVE THIS FILE IN THE DEFAULT PACKAGE
//  (i.e., DO NOT add 'package cs311.pa1;' or similar)

// DO NOT MODIFY THE EXISTING METHOD SIGNATURES
//  (you may, however, add member fields and additional methods)

// DO NOT INCLUDE LIBRARIES OUTSIDE OF THE JAVA STANDARD LIBRARY
//  (i.e., you may only include libraries of the form java.*)

/**
* @author Christian Shinkle, Alec Harrison, Benjamin Trettin
*/

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Map.Entry;
import java.util.PriorityQueue;
import java.util.Queue;
import java.util.Scanner;
import java.util.Set;
import java.util.Stack;

public class NetworkInfluence {

	String[] masterList;
	HashMap<String, ArrayList<String>> graph;

	// NOTE: graphData is an absolute file path that contains graph data, NOT
	// the
	// raw graph data itself
	@SuppressWarnings("resource")
	public NetworkInfluence(String graphData) {
		graph = new HashMap<String, ArrayList<String>>();
		File graphFile = new File(graphData);
		Scanner reader = null;
		try {
			reader = new Scanner(graphFile);
		} catch (IOException e) {
			System.out.println("File " + graphData + " was not found.");
			return;
		}

		String tmp = reader.nextLine();
		int size = Integer.parseInt(tmp);
		masterList = new String[size];

		for (int count = 1, index = 0; reader.hasNextLine(); count++) {
			String line = reader.nextLine();
			String[] parts = line.split(" ");
			if (parts.length < 2) {
				throw new RuntimeException("Too few arguements on line: " + count);
			}

			String key = parts[0];
			String value = parts[1];

			boolean isKeyDup = false;
			boolean isValDup = false;

			for (int i = 0; i < index; i++) {
				if (masterList[i].equals(key)) {
					isKeyDup = true;
				}
				if (masterList[i].equals(value)) {
					isValDup = true;
				}
			}

			if (!isKeyDup) {
				masterList[index++] = key;
			}

			if (!isValDup) {
				masterList[index++] = value;
			}

			if (!graph.containsKey(key)) {
				ArrayList<String> edges = new ArrayList<String>();
				edges.add(value);
				graph.put(key, edges);
			} else {
				ArrayList<String> edges2 = graph.get(key);
				if (!edges2.contains(value)) {
					edges2.add(value);
				}
			}

		}
		reader.close();
	}

	public int outDegree(String v) {
		if (graph.get(v) != null)
			return graph.get(v).size();
		return 0;
	}

	public ArrayList<String> shortestPath(String u, String v) {
		ArrayList<String> result = new ArrayList<>();
		Queue<String> queue = new LinkedList<>();
		HashMap<String, Integer> dist = new HashMap<>();
		HashMap<String, String> prev = new HashMap<>();

		for (String node : masterList)
			dist.put(node, Integer.MAX_VALUE);

		dist.replace(u, 0);
		queue.add(u);

		while (!queue.isEmpty()) {
			String current = queue.poll();

			if (current.equals(v)) {
				buildPath(current, result, prev);
				return result;
			}

			ArrayList<String> outVertices = graph.get(current);

			if (outVertices == null)
				continue;

			for (String neighbor : outVertices) {
				int alt = dist.get(current) + 1;
				if (alt < dist.get(neighbor)) {
					dist.replace(neighbor, alt);
					prev.put(neighbor, current);
					queue.add(neighbor);
				}
			}
		}
		return result;
	}

	private void buildPath(String current, ArrayList<String> result, HashMap<String, String> prev) {
		Stack<String> stack = new Stack<String>();
		String s = current;

		while (s != null) {
			stack.push(s);
			s = prev.get(s);
		}

		while (!stack.isEmpty()) {
			result.add(stack.pop());
		}

	}

	public int distance(String u, String v) {
		ArrayList<String> list = shortestPath(u, v);
		int distance = list.size() - 1;
		return (distance != 0) ? distance : -1;
	}

	public int distance(ArrayList<String> s, String v) {
		int minDist = Integer.MAX_VALUE;
		for (int i = 0; i < s.size(); i++) {
			int current = distance(s.get(i), v);
			if (current != -1 && current < minDist) {
				minDist = current;
			}
		}
		return (minDist == Integer.MAX_VALUE) ? -1 : minDist;
	}

	public float influence(String u) {
		HashMap<String, ArrayList<String>> MST = buildMST(u);
		float result = influenceHelper(u, .5, MST);
		result = result + 1;
		return result;

	}

	// TODO
	// change this to private before submitting
	public HashMap<String, ArrayList<String>> buildMST(String vertex) {
		HashMap<String, ArrayList<String>> MST = new HashMap<>();
		HashMap<String, String> visited = new HashMap<>();
		Queue<String> queue = new LinkedList<>();
		visited.putIfAbsent(vertex, vertex);
		queue.add(vertex);
		while (!queue.isEmpty()) {
			String current = queue.poll();
			ArrayList<String> children = graph.get(current);
			ArrayList<String> newChildren = new ArrayList<String>();
			MST.put(current, newChildren);
			if (children != null) {
				for (String child : children) {
					if (!visited.containsKey(child)) {
						newChildren.add(child);
						visited.putIfAbsent(child, child);
						queue.add(child);
					}
				}
			}
		}
		return MST;
	}

	private float influenceHelper(String vertex, double denominator, HashMap<String, ArrayList<String>> MST) {
		int size = MST.get(vertex).size();
		double influenceTotal = denominator * size;
		ArrayList<String> children = MST.get(vertex);
		for (int i = 0; i < size; i++) {
			String child = children.get(i);
			influenceTotal += influenceHelper(child, denominator / 2, MST);
		}
		return (float) influenceTotal;
	}

	public float influence(ArrayList<String> s) {
		// implementation
		HashMap<String, String> GivenStringsMap = new HashMap<>();
		int current = Integer.MAX_VALUE;
		float runningTotal = 0;
		for (String givenString : s) {
			GivenStringsMap.put(givenString, givenString);
		}
		for (String masterString : masterList) {
			if (!GivenStringsMap.containsKey(masterString)) {
				current = distance(s, masterString);
				// If no path exists Currentlowest isn't updated
				if (current > 0) {
					runningTotal += 1.0 / power(2, current);
				}
			}
		}
		// Added this to account for the influental node on themselves
		runningTotal = runningTotal + s.size();
		return runningTotal;
	}

	private int power(int x, int y) {
		if (y == 0)
			return 1;
		else if (y % 2 == 0)
			return power(x, y / 2) * power(x, y / 2);
		else
			return x * power(x, y / 2) * power(x, y / 2);
	}

	public ArrayList<String> mostInfluentialDegree(int k) {
		ArrayList<String> list = new ArrayList<String>();
		PriorityQueue<Tuple> pq = new PriorityQueue<Tuple>(k, new Comparator<Tuple>() {
			public int compare(Tuple lhs, Tuple rhs) {
				if (lhs.dist <= rhs.dist)
					return 1;
				else if (lhs.dist > rhs.dist)
					return -1;
				return 0;
			}
		});
		for (int i = 0; i < masterList.length; i++) {
			Tuple element = new Tuple(masterList[i], outDegree(masterList[i]));
			pq.add(element);
		}
		for (int i = 0; i < k; i++) {
			String s = pq.poll().string;
			list.add(s);
		}
		return list;
	}

	public ArrayList<String> mostInfluentialModular(int k) {
		ArrayList<String> list = new ArrayList<String>();
		PriorityQueue<modularTuple> pq = new PriorityQueue<modularTuple>(k, new Comparator<modularTuple>() {
			public int compare(modularTuple lhs, modularTuple rhs) {
				if (lhs.dist <= rhs.dist)
					return 1;
				else if (lhs.dist > rhs.dist)
					return -1;
				return 0;
			}
		});
		for (int i = 0; i < masterList.length; i++) {
			modularTuple element = new modularTuple(masterList[i], influence(masterList[i]));
			pq.add(element);
		}
		for (int i = 0; i < k; i++) {
			String s = pq.poll().string;
			list.add(s);
		}
		return list;

	}

	public ArrayList<String> mostInfluentialSubModular(int k) {
		ArrayList<String> list = new ArrayList<String>();
		boolean[] masterTemp = new boolean[masterList.length];
		for (int i = 0; i < masterList.length; i++)
			masterTemp[i] = true;

		// do the loop k times
		for (int x = 0; x < k; x++) {
			PriorityQueue<subModularTuple> pq = new PriorityQueue<subModularTuple>(k,
					new Comparator<subModularTuple>() {
						public int compare(subModularTuple lhs, subModularTuple rhs) {
							if (lhs.dist <= rhs.dist)
								return 1;
							else if (lhs.dist > rhs.dist)
								return -1;
							return 0;
						}
					});
			
			ArrayList<String> vList = new ArrayList<String>();
			for (int j = 0; j < list.size(); j++) {
				vList.add(list.get(j));
			}
			
			for (int i = 0; i < masterTemp.length; i++) {
				if (masterTemp[i]) {
					vList.add(masterList[i]);
					subModularTuple vInf = new subModularTuple(masterList[i], influence(vList), i);
					pq.add(vInf);
					vList.remove(vList.size()-1);
				}
			}

			// add found vertice to the list and remove it from masterTemp.
			subModularTuple vertice = pq.poll();
			String s = vertice.string;
			int index = vertice.index;
			masterTemp[index] = false;
			list.add(s);
		}
		return list;
	}

	private class Tuple implements Comparable<Tuple> {

		String string;
		int dist;

		public Tuple(String s, int i) {
			string = s;
			dist = i;
		}

		@Override
		public int compareTo(Tuple other) {
			if (this.dist < other.dist)
				return -1;
			else if (this.dist > other.dist)
				return 1;
			else
				return 0;
		}

		@Override
		public String toString() {
			return "(" + string + ", " + dist + ")";

		}

	}

	private class modularTuple implements Comparable<modularTuple> {

		String string;
		float dist;

		public modularTuple(String s, float i) {
			string = s;
			dist = i;
		}

		@Override
		public int compareTo(modularTuple other) {
			if (this.dist < other.dist)
				return -1;
			else if (this.dist > other.dist)
				return 1;
			else
				return 0;
		}

		@Override
		public String toString() {
			return "(" + string + ", " + dist + ")";

		}

	}

	private class subModularTuple implements Comparable<subModularTuple> {

		String string;
		float dist;
		int index;

		public subModularTuple(String s, float i, int in) {
			string = s;
			dist = i;
			index = in;
		}

		@Override
		public int compareTo(subModularTuple other) {
			if (this.dist < other.dist)
				return -1;
			else if (this.dist > other.dist)
				return 1;
			else
				return 0;
		}

		@Override
		public String toString() {
			return "(" + string + ", " + dist + ")";

		}

	}

}

