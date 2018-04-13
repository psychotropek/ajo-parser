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
					//get suffix of file for number (figure1, figure2, etc)
					//for excel files, getRaw, then call the API. Pass the html into an array. 
					//todo: stringfind, get everything starting figure, and then continue via substring
					$figure = '';
					$item['field'] = '';
				break;
				case "png":
				case "jpg":
				case "jpeg":
					if ($path_parts['filename'] == 'medium_image' || $path_parts['filename'] == 'big_image') {
						$item[$path_parts['filename']]['file'] = 0;
						$item[$path_parts['filename']]['caption'] = 0;
					}
					else {
						//$path_parts['filename'] == ? figure 1 etc
					}
					//get suffix of file for number, same as xls (figure1, figure2, figure3, small image, large image, etc)
					$figure_array = new Array();
					$figure_array['name'] = 'f2';
					$item['path'] = $file;


					$item['figures'][$figure] =  
					
					//small image or large image, handle as just item path
					//todo: implement
					$item['field'] = '';
					//todo: stringfind, get everything starting figure, and then continue via substring
					//otherwise for image files, get some metadata (exif? or do we need to get raw?) for caption
					
					$item['caption'] = exif_read_data($file, 'exif');
					//todo: find how figures are ordered in the content
					$item['number'] = '';

					$item[$figure] = "image";
					//todo: ask Chris what exif data it will be kept in- send a sample image

					//todo: in some cases this will go to the side for figures
				break;
			}
			$result->items[] = $item;
		}

		return $result;
	}

}
	