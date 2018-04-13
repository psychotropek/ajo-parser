<?php

/**
 * @file
 * Contains the FeedsAJOParser class.
 */

class FeedsAJOParser extends FeedsParser {

	/**
	* Implements FeedsParser::parse().
	*/

	public function parse(FeedsSource $source, FeedsFetcherResult $fetcher_result) {

		require DRUPAL_ROOT.'/sites/all/vendor/autoload.php';

		$result = new FeedsParserResult();
		$files=$fetcher_result->file_list;
	//use module name, then folder. may already be built-in.
	//todo: echo?
	//array(2) { [0]=> string(57) "public://feeds/ajo_test_2/testzip_9/amp_um_flower.jpeg" 
	//[1]=> string(63) "public://feeds/ajo_test_2/testzip_9/book reviews part 2.docx" }

		//foreach file, get the extension
		foreach ($files as $file) {
			$item = array();

			$path_parts = pathinfo($file); //check for result
			$file_type = $path_parts['extension'];
			
			switch($file_type) {
				case "doc":
				case "docx":
					try {
						$client = new GuzzleHttp\Client(['base_uri' => 'https://api.docconverter.pro','verify' => false]);
						$res = $client->post('/Token', [ 'form_params' => [ 'grant_type' => 'password', 'username' => 'chris@quillandcode.com', 'password' => 'AJO_d3v070p'] ]);
						$statusCode = $res->getStatusCode();
						if ( $statusCode != 200 )
						{
							echo 'Invalid request status code: ' . $statusCode;
							die();
						}
						$body = (string)$res->getBody();
						$data = json_decode($body);
						$token = $data->access_token;
						// $token can be stored in site cache or database, please check expiry date ($data->expires_in) before using token again (2 weeks expiration time)

						$formData = array();
						$formData[] = [ 'name' => 'file_name', 'contents' => fopen($file, 'r')];
						$formData[] = [ 'name' => 'template', 'contents' => 'AJO test template'];
						$formData[] = [ 'name' => 'returnHtml', 'contents' => 'true'];
						$formData[] = [ 'name' => 'returnData', 'contents' => 'true'];
						
						$response = $client->request('POST', '/api/converter/convertdoc', [
							'headers' => [ 'Authorization' => 'Bearer ' . $token ],
							'multipart' => $formData,
						]);
                    	
						$test = $response->getBody();
						$handle = (string)$test;
						$handle = preg_replace("%\t%","",$handle);
						$handle = preg_replace("%<p>(\s)*&#xa0;(\s)*</p>%","",$handle);

						preg_match_all("%<h1>(.*)</h1>%",$handle,$title,PREG_PATTERN_ORDER);
						$item['title'] = $title;

						preg_match_all("%<h6>.*Author.*</h6>\n(.*)\n%",$handle,$authors_match,PREG_PATTERN_ORDER);
						$items['authors'] = $authors_match;

						preg_match_all("%<h2>\s*Summary\s*</h2>(((?!<h)(.|\n))*)%",$handle,$summary_match,PREG_PATTERN_ORDER);
						$item['summary'] = $summary_match;

						preg_match_all("%<h2>\s*Teaser\s*</h2>(((?!<h)(.|\n))*)%",$handle,$teaser_match,PREG_PATTERN_ORDER);
						$item['teaser'] = $teaser_match;

						preg_match_all("%<h2>\s*Take-Home Points\s*</h2>(((?!<h)(.|\n))*)%",$handle,$points_match,PREG_PATTERN_ORDER);
						$item['take_home_points'] = $points_match;						

						preg_match_all("%</ul>(\s|n)*<h2>(\s|.)*</h2>((?!<h)(.|\n))*%",$handle,$body_match,PREG_PATTERN_ORDER);
						$item['body'] = $body_match;
						
						//TODO: handle References, Topics, Authors, and Sections as lists. here or processor?
						preg_match_all("%<h2>\s*References\s*</h2>(((?!<h)(.|\n))*)%",$handle,$ref_match,PREG_PATTERN_ORDER);
						$item['references'] = $ref_match;

						preg_match_all("%<h6>\s*Topics\s*</h6>(((?!<h)(.|\n))*)%",$handle,$topics_match,PREG_PATTERN_ORDER);
						$item['topics'] = $topics_match;

						preg_match_all("%<h6>\s*Sections\s*</h6>(((?!<h)(.|\n))*)%",$handle,$sections_match,PREG_PATTERN_ORDER);
						$item['sections'] = $sections_match;

						preg_match_all("%<h6>\s*Digital Object ID\s*</h6>(((?!<h)(.|\n))*)%",$handle,$doi_match,PREG_PATTERN_ORDER);
						$item['digital_object_id'] = $doi_match;

					}
					catch (Exception $exc) {
						echo $exc->getMessage();
					}

				break;
				case "xls":
				case "xslx":
					//todo: get index from filename
					$table_index = 1;
					try {
						$client = new GuzzleHttp\Client(['base_uri' => 'https://api.docconverter.pro','verify' => false]);
						$res = $client->post('/Token', [ 'form_params' => [ 'grant_type' => 'password', 'username' => 'chris@quillandcode.com', 'password' => 'AJO_d3v070p'] ]);
						$statusCode = $res->getStatusCode();
						if ( $statusCode != 200 )
						{
							echo 'Invalid request status code: ' . $statusCode;
							die();
						}
						$body = (string)$res->getBody();
						$data = json_decode($body);
						$token = $data->access_token;
						// $token can be stored in site cache or database, please check expiry date ($data->expires_in) before using token again (2 weeks expiration time)
		//todo: excel template? test
						$formData = array();
						$formData[] = [ 'name' => 'file_name', 'contents' => fopen($file, 'r')];
						$formData[] = [ 'name' => 'template', 'contents' => 'AJO test template'];
						$formData[] = [ 'name' => 'returnHtml', 'contents' => 'true'];
						$formData[] = [ 'name' => 'returnData', 'contents' => 'true'];
						
						$response = $client->request('POST', '/api/converter/convertdoc', [
							'headers' => [ 'Authorization' => 'Bearer ' . $token ],
							'multipart' => $formData,
						]);
                    	
						$test = $response->getBody();
						$handle = (string)$test;
						$table_array = array();
						//todo: there will be a heading, but it'll be at the top
						//todo: figure out caption and put it in that field
						$item['tables'][$table_index] = $table_array;
					}
					catch (Exception $exc) {
						echo $exc->getMessage();
					}
				break;
				case "png":
				case "jpg":
				case "jpeg":
					if ($path_parts['filename'] == 'medium_image' || $path_parts['filename'] == 'big_image') {
						$image_path = drupal_get_path('module', 'ajo_test').$path_parts['filename'];
						$item[$path_parts['filename']]['filepath'] = $image_path;
						$image_xml = exif_custom_get_xmp($image_path);
						$item[$path_parts['filename']]['caption'] = $image_xml['XMP:xmpmeta:rdf:description:description:alt:li'];
					}
					else {
						$figure_array = array();
						//todo: figure index? get last numerical digits of filename?
						$figure_index = 1;
						$image_path = drupal_get_path('module', 'ajo_test').$path_parts['filename'];
						$figure_array['filepath'] = $image_path;
						$image_xml = exif_custom_get_xmp($image_path);
						$figure_array['caption'] = $image_xml['XMP:xmpmeta:rdf:description:description:alt:li'];

						$item['figures'][$figure_index] = $figure_array; 

					}

				break;
			}
			$result->items[] = $item;
		}

		return $result;
	}

	function exif_custom_get_xmp($image) {
		$content = file_get_contents($image);
		$xmp_data_start = strpos($content, '<x:xmpmeta');
		$xmp_data_end = strpos($content, '</x:xmpmeta>');
		if ($xmp_data_start === FALSE || $xmp_data_end === FALSE) {
		  return array();
		}
		$xmp_length = $xmp_data_end - $xmp_data_start;
		$xmp_data = substr($content, $xmp_data_start, $xmp_length + 12);
		unset($content);
		$xmp = simplexml_load_string($xmp_data);
		if ($xmp === FALSE) {
		  return array();
		}
		// $namespaces = $xmp->getDocNamespaces(true);
		// $fields = array();
		// foreach ($namespaces as $namespace) {
		// $fields[] = exif_custom_xml_recursion($xmp->children($namespace));
		$field_data = array();
		exif_custom_xml_recursion($xmp, $field_data, 'XMP');
	   
		return $field_data;
	  }
	  
	  function exif_custom_xml_recursion($obj, array &$fields, $name) {
		$namespace = $obj->getDocNamespaces(TRUE);
		$namespace[NULL] = NULL;
	  
		$children = array();
		$attributes = array();
	  
		$text = trim((string) $obj);
		if (strlen($text) === 0) {
		  $text = NULL;
		}
	  
		if (strtolower((string) $obj->getName()) == "bag") {
		  // @todo Add support for bags of objects other than just text?
		  $childValues = array();
		  $objChildren = $obj->children("rdf", TRUE);
		  foreach ($objChildren as $child) {
			$childValues[] = trim((string) $child);
		  }
		  if (count($childValues) > 0) {
			$fields[$name] = $childValues;
		  }
		}
		else {
		  $name = $name . ':' . strtolower((string) $obj->getName());
	  
		  // Get info for all namespaces.
		  if (is_object($obj)) {
			foreach ($namespace as $ns => $nsUrl) {
			  // Attributes.
			  $objAttributes = $obj->attributes($ns, TRUE);
			  foreach ($objAttributes as $attributeName => $attributeValue) {
				$attribName = strtolower(trim((string) $attributeName));
				$attribVal = trim((string) $attributeValue);
				if (!empty($ns)) {
				  $attribName = $ns . ':' . $attribName;
				}
				$attributes[$attribName] = $attribVal;
			  }
	  
			  // Children.
			  $objChildren = $obj->children($ns, TRUE);
			  foreach ($objChildren as $childName => $child) {
				$childName = strtolower((string) $childName);
				if (!empty($ns)) {
				  $childName = $ns . ':' . $childName;
				}
				$children[$childName][] = exif_custom_xml_recursion($child, $fields, $name);
			  }
			}
		  }
		  if (!is_null($text)) {
			$fields[$name] = $text;
		  }
		}
	  
		return array(
		  'name' => $name,
		  'text' => html_entity_decode($text),
		  'attributes' => $attributes,
		  'children' => $children,
		);
	  }
	

}
	