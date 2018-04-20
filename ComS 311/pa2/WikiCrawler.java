// LEAVE THIS FILE IN THE DEFAULT PACKAGE
//(i.e., DO NOT add 'package cs311.pa1;' or similar)

//DO NOT MODIFY THE EXISTING METHOD SIGNATURES
//(you may, however, add member fields and additional methods)

//DO NOT INCLUDE LIBRARIES OUTSIDE OF THE JAVA STANDARD LIBRARY
//(i.e., you may only include libraries of the form java.*)

/**
* @author Christian Shinkle, Alec Harrison, Benjamin Trettin
*/

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;
import java.util.Scanner;

/**
 * 
 * @author Benjamin Trettin, Christian Shinkle, Alec Harrison
 *
 */
public class WikiCrawler {

	public static final String BASE_URL = "https://en.wikipedia.org";

	private int max, requests;

	private String seedUrl, fileName;

	private ArrayList<String> topics;

	private HashMap<String, String> foundGoodLinks, foundBadLinks;

	public WikiCrawler(String seedUrl, int max, ArrayList<String> topics, String fileName) {
		this.max = max;
		this.seedUrl = seedUrl;
		this.topics = topics;
		this.fileName = fileName;
		foundGoodLinks = new HashMap<>();
		foundBadLinks = new HashMap<>();
		foundGoodLinks.put(seedUrl, seedUrl);
	}

	public void crawl() {
		LinkedHashMap<String, ArrayList<String>> graph = new LinkedHashMap<String, ArrayList<String>>();
		if (hasTopics(seedUrl)) {
			Queue<String> queue = new LinkedList<String>();
			HashMap<String, String> visited = new HashMap<>();
			queue.add(seedUrl);
			visited.put(seedUrl, seedUrl);
			while (!queue.isEmpty()) {
				String curPage = queue.poll();
				String curPageHTML = fetchPage(curPage);
				ArrayList<String> pageLinks = findLinks(curPageHTML, curPage);
				for (String link : pageLinks) {
					if (!visited.containsKey(link)) {
						queue.add(link);
					}
					visited.putIfAbsent(link, link);
				}
				graph.put(curPage, pageLinks);
			}
		}
		writeToFile(graph);
	}

	/**
	 * Checks if the actual text component contains all of the topics
	 * 
	 * @param url
	 * @return
	 */
	private boolean hasTopics(String url) {
		if (topics.size() == 0)
			return true;
		String subHTML = fetchPage(url);
		ArrayList<String> topix = new ArrayList<String>();
		topix.addAll(topics);
		Scanner scan = new Scanner(subHTML);
		while (scan.hasNextLine()) {
			String next = scan.nextLine();
			for (int i = 0; i < topix.size(); i++) {
				if (next.contains(topix.get(i))) {
					topix.set(i, null);
				}
			}
		}
		scan.close();
		for (int i = 0; i < topix.size(); i++) {
			if (topix.get(i) != null) {
				return false;
			}
		}
		return true;
	}

	/**
	 * returns all of the valid links in the ?actual text component? of the
	 * current page.
	 * 
	 * @param subHTML
	 * @param url
	 * @return
	 */
	private ArrayList<String> findLinks(String subHTML, String url) {
		ArrayList<String> links = new ArrayList<String>();
		HashMap<String, String> linksLookUpTable = new HashMap<>();
		Scanner scan = new Scanner(subHTML);
		String wiki = "/wiki/";
		String href = "href";
		String org = ".org";
		while (scan.hasNext()) {
			String next = scan.next();
			int startIndex = 0;
			int endIndex = 0;
			if (next.contains(wiki) && next.contains(href)) {
				for (int i = 0; i < next.length(); i++) {
					if (next.charAt(i) == '"' && next.charAt(i - 1) == '=') {
						startIndex = i + 1;
					}
					if (next.charAt(i) == '"' && next.charAt(i - 1) != '=') {
						endIndex = i;
						break;
					}
				}

				String possibleLink = next.substring(startIndex, endIndex);

				if (!possibleLink.contains("#") && !possibleLink.contains(":") && !possibleLink.contains(org)
						&& !linksLookUpTable.containsKey(possibleLink) && !possibleLink.equals(url)) {
					if (foundBadLinks.containsKey(possibleLink)) {
						// do nothing
					} else if (foundGoodLinks.size() < max) {
						if (!foundGoodLinks.containsKey(possibleLink)) {
							// EXPENSIVE CALL
							if (hasTopics(possibleLink)) {
								foundGoodLinks.put(possibleLink, possibleLink);
								links.add(possibleLink);
								linksLookUpTable.put(possibleLink, possibleLink);
							} else {
								foundBadLinks.put(possibleLink, possibleLink);
							}
						} else {
							links.add(possibleLink);
							linksLookUpTable.put(possibleLink, possibleLink);
						}
					} else if (foundGoodLinks.size() == max) {
						if (foundGoodLinks.containsKey(possibleLink)) {
							links.add(possibleLink);
							linksLookUpTable.put(possibleLink, possibleLink);
						}
					}
				}
			}
		}
		scan.close();
		return links;
	}

	/**
	 * makes a request to the server to fetch html of the current page and
	 * creates a string for the actual text component of the page.
	 * 
	 * @param currentPage
	 * @return
	 */
	private String fetchPage(String currentPage) {
		requests++;
		int mod = requests % 25;
		if (mod == 0) {
			try {
				Thread.sleep(3000);
			} catch (Exception e) {
				System.out.println("There was an exception thrown while " + "trying to sleep.");
				System.exit(1);
			}
		}
		URL url = null;
		InputStream is = null;
		try {
			url = new URL(BASE_URL + currentPage);
			is = url.openStream();
		} catch (MalformedURLException e) {
			System.out.println("There was an exception thrown while " + "trying to get page.");
			System.exit(1);
		} catch (IOException e) {
			System.out.println("There was an exception thrown while " + "trying to get page.");
			System.exit(1);
		}
		BufferedReader br = new BufferedReader(new InputStreamReader(is));
		String input;
		StringBuilder builder = new StringBuilder();
		try {
			while ((input = br.readLine()) != null) {
				builder.append(input);
			}
			br.close();
		} catch (IOException e) {
			System.out.println("There was an exception thrown while " + "trying to read input from the BufferedReader");
			System.exit(1);
		}
		String HTML = builder.toString();

		// Create subString starting after first <p>
		String p1 = "<p>";
		int startIndex = HTML.indexOf(p1);
		String subHTML = HTML.substring(startIndex, HTML.length());
		return subHTML;
	}

	/**
	 * Takes the graph and saves it to a file by listing out all of the edges.
	 * @param graph
	 */
	private void writeToFile(LinkedHashMap<String, ArrayList<String>> graph) {
		System.out.println(fileName);
		PrintWriter printWriter = null;
		try {
			printWriter = new PrintWriter(fileName, "UTF-8");
		} catch (Exception e) {
			System.out.println("There was an exception thrown while " + "trying to write to" + fileName + ".");
			System.exit(1);
		}
		printWriter.println(foundGoodLinks.size());
		for (Map.Entry<String, ArrayList<String>> entry : graph.entrySet()) {
			String key = entry.getKey();
			ArrayList<String> value = entry.getValue();
			for (int i = 0; i < value.size(); i++) {
				String vertice = value.get(i);
				printWriter.print(key + " " + vertice);
				printWriter.println();
			}
		}
		printWriter.close();
	}

}
